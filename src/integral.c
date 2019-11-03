
/*
 
 Copyright (c) 2019 Hannes Eberhard
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

#include "symbolic4.h"

bool expression_is_risch_integrable(expression* source, expression* variable) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (!expression_is_risch_integrable(source->children[i], variable)) return false;
    }
    
    if (source->identifier == EXPI_EXPONENTATION && source->children[0]->value.symbolic[0] != 'e' && count_occurrences(source->children[1], variable, true) != 0) {
        return false;
    } else if (source->identifier == EXPI_EXPONENTATION && count_occurrences(source->children[0], variable, true) != 0 && source->children[1]->identifier != EXPI_LITERAL) {
        return false;
    }  else if (source->identifier == EXPI_EXPONENTATION && count_occurrences(source->children[0], variable, true) != 0 && source->children[1]->value.numeric.denominator != 1) {
        return false;
    } else if (source->identifier == EXPI_LOG) {
        return false;
    }
    
    return true;
    
}

void risch_get_extensions(expression* extensions, expression* source, expression* variable) {
    
    uint8_t i;
    expression* extension;
    
    if (count_occurrences(source, variable, true) == 0) {
        return;
    }
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        risch_get_extensions(extensions, source->children[i], variable);
    }
    
    if (source->identifier == EXPI_EXPONENTATION && source->children[0]->value.symbolic[0] == 'e') {
        extension = new_expression(EXPT_STRUCTURE, EXPI_EXTENSION, 2,
                                   new_symbol(EXPI_SYMBOL, "EE____"),
                                   copy_expression(source->children[1]));
    } else if (source->identifier == EXPI_LN) {
        extension = new_expression(EXPT_STRUCTURE, EXPI_EXTENSION, 2,
                                   new_symbol(EXPI_SYMBOL, "LE____"),
                                   copy_expression(source->children[1]));
    } else {
        return;
    }
    
    itoa(extension->children[0]->value.symbolic + 2, extensions->child_count);
    extension->children[0]->value.symbolic[strlen(extension->children[0]->value.symbolic)] = '_';
    append_child(extensions, extension);
    
    replace_expression(source, copy_expression(extension->children[0]));
    
}

uint8_t risch_determine_parts(expression** polynominal_part, expression** rational_part, const expression* source, const expression* variable, const expression* extensions) {
    
    uint8_t i;
    expression* quotient;
    expression* remainder;
    expression* temp;
    
    if (expression_is_reziprocal(source)) {
        
        *polynominal_part = NULL;
        
        *rational_part = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                        new_literal(1, 1, 1),
                                        copy_expression(source));
        

        (*rational_part)->children[1]->children[1]->sign = 1;
        
        simplify(*rational_part, true);
        
        ERROR_CHECK(any_expression_to_sparse_polynomial((*rational_part)->children[0], variable));
        ERROR_CHECK(any_expression_to_sparse_polynomial((*rational_part)->children[1], variable));
        
    } else if (source->identifier == EXPI_MULTIPLICATION) {
        
        temp = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                              new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0),
                              new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0));
        
        for (i = 0; i < source->child_count; i++) {
            if (expression_is_reziprocal(source->children[i])) {
                append_child(temp->children[1], copy_expression(source->children[i]));
                temp->children[1]->children[temp->children[1]->child_count - 1]->children[1]->sign = 1;
            } else {
                append_child(temp->children[0], copy_expression(source->children[i]));
            }
        }
        
        simplify(temp, true);
        
        ERROR_CHECK(any_expression_to_sparse_polynomial(temp->children[0], variable));
        ERROR_CHECK(validate_sparse_polynomial(temp->children[0], false, false, false));
        
        ERROR_CHECK(any_expression_to_sparse_polynomial(temp->children[1], variable));
        ERROR_CHECK(validate_sparse_polynomial(temp->children[1], false, false, false));
        
        poly_div(&quotient, &remainder, temp->children[0], temp->children[1], -1);
        
        *polynominal_part = quotient;
        *rational_part = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                        remainder,
                                        temp->children[1]);
        
    } else {
        
        *polynominal_part = copy_expression(source);
        *rational_part = NULL;
        ERROR_CHECK(any_expression_to_sparse_polynomial(*polynominal_part, variable));
        ERROR_CHECK(validate_sparse_polynomial(*polynominal_part, false, false, false));
        
    }
    
    return RETS_SUCCESS;
    
}

void risch_integrate_polynominal_part(expression* source) {
    
    uint8_t i;
    expression* exponent;
    expression* result;
    
    if (source == NULL) return;
    
    result = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    
    for (i = 0; i < source->child_count; i++) {
        
        exponent = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                  copy_expression(source->children[i]->children[0]),
                                  new_literal(1, 1, 1));
        
        simplify(exponent, true);
        
        append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                            copy_expression(exponent),
                                            new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                           copy_expression(source->children[i]->children[1]),
                                                                          copy_expression(exponent)),
                                            copy_expression(source->children[i]->children[2])));
        
        free_expression(exponent, false);
        
    }
    
    simplify(result, true);
    replace_expression(source, result);
    
}

void rothstein_trager_method(expression* source) {
    
    uint8_t i, j;
    expression* a = copy_expression(source->children[0]);
    uint8_t a_degree;
    expression* b = copy_expression(source->children[1]);
    expression* b_derivative;
    expression* b_derivative_negative;
    expression* a_coefficients = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* b_coefficients = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* z = new_symbol(EXPI_SYMBOL, "EZ");
    expression* extentions = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* resultant_poly;
    expression* resultant;
    expression* primitive_part;
    expression* factor;
    uint8_t factor_degree;
    expression* equation;
    expression* temp;
    expression* gcd;
    expression* result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    derivative(&b_derivative, b, NULL, true);
    b_derivative_negative = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                           copy_expression(b_derivative),
                                           new_literal(-1, 1, 1));
    simplify(b_derivative_negative, true);
    
    simplify(b, true);
    any_expression_to_sparse_polynomial(b, NULL);
    any_expression_to_sparse_polynomial(b_derivative, NULL);
    any_expression_to_sparse_polynomial(b_derivative_negative, NULL);
    
    append_child(extentions, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                            copy_expression(z),
                                            copy_expression(z)));
    
    resultant_poly = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_DENSE, 2,
                                    copy_expression(z),
                                    new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                                   copy_expression(a),
                                                   copy_expression(b_derivative_negative)));
    
    any_expression_to_expression(resultant_poly->children[1]->children[0]);
    any_expression_to_expression(resultant_poly->children[1]->children[1]);
    
    a_degree = max(a->children[0]->children[0]->value.numeric.numerator, b_derivative->children[0]->children[0]->value.numeric.numerator);
    
    any_expression_to_dense_polynomial(a, NULL);
    any_expression_to_dense_polynomial(b, NULL);
    any_expression_to_dense_polynomial(b_derivative_negative, NULL);
    
    for (i = 0; i < a_degree + 1; i++) {
        append_child(a_coefficients, new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_DENSE, 2,
                                                    copy_expression(z),
                                                    new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                                                   copy_expression(a->children[1]->children[i]),
                                                                   copy_expression(b_derivative_negative->children[1]->children[i]))));
    }
    
    for (i = 0; i < b->children[1]->child_count; i++) {
        append_child(b_coefficients, new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_DENSE, 2,
                                                    copy_expression(z),
                                                    new_expression(EXPT_STRUCTURE, EXPI_LIST, 1,
                                                                   copy_expression(b->children[1]->children[i]))));
    }
    
    calculate_resultant(&resultant, a_coefficients, b_coefficients);
    simplify(resultant, true);
    primitive_part = copy_expression(resultant);
    
    if (primitive_part != NULL) {
        make_monic(primitive_part);
    }
    
    factor_square_free(primitive_part);
    
    for (i = 0; i < primitive_part->child_count; i++) {
        
        factor = primitive_part->children[0];
        
        if (literal_to_double(factor->children[0]) == 1) {
            continue;
        }
        
        factor_degree = factor->children[0]->children[1]->child_count - 1;
        
        if (factor_degree <= 2) {
            
            any_expression_to_expression_recursive(factor->children[0]);
            any_expression_to_expression_recursive(a);
            any_expression_to_expression_recursive(b);
            any_expression_to_expression_recursive(b_derivative);
            
            equation = new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                      factor->children[0],
                                      new_literal(1, 0, 1));
            solve(equation, NULL);
            embed_in_list_if_necessary(equation);
            
            for (j = 0; j < equation->child_count; j++) {
                
                temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                      new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                                     new_literal(-1, 1, 1),
                                                     copy_expression(equation->children[j]->children[1]),
                                                     copy_expression(b_derivative)),
                                      copy_expression(a));
                
                simplify(temp, true);
                
                any_expression_to_sparse_polynomial(temp, NULL);
                poly_gcd(&gcd, temp, copy_expression(source->children[1]));
                any_expression_to_expression_recursive(gcd);
                
                append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                    new_expression(EXPT_FUNCTION, EXPI_LN, 1, copy_expression(gcd)),
                                                    copy_expression(equation->children[j]->children[1])));
                
                
            }
            
        }
        
    }
    
    simplify(result, true);
    
    replace_expression(source, result);
    
}

uint8_t risch_integrate_rational_part(expression* source) {
    
    if (source == NULL) return RETS_UNCHANGED;
    
    if (poly_is_square_free(source->children[1])) {
        rothstein_trager_method(source);
        return RETS_CHANGED;
    } else {
        return RETS_UNCHANGED;
    }
    
}

uint8_t risch_integrate(expression* source, expression* variable) {
    
    expression* polynominal_part;
    expression* rational_part;
    expression* extensions = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* last_extension;
    expression* temp_source = copy_expression(source);
    
    if (!expression_is_risch_integrable(source, variable)) return RETS_UNCHANGED;
    
    risch_get_extensions(extensions, temp_source, variable);
    last_extension = (extensions->child_count) ? copy_expression(extensions->children[extensions->child_count - 1]) : NULL;
    
    ERROR_CHECK(risch_determine_parts(&polynominal_part, &rational_part, temp_source, (last_extension) ? last_extension->children[0] : variable, extensions));
    
    if (extensions->child_count == 0) {
        risch_integrate_polynominal_part(polynominal_part);
        risch_integrate_rational_part(rational_part);
    } else {
        return RETS_UNCHANGED;
    }

    if (polynominal_part == NULL) {
        replace_expression(source, rational_part);
    } else if (rational_part == NULL) {
        replace_expression(source, polynominal_part);
    } else {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                  polynominal_part,
                                                  rational_part));
    }
    
    simplify(source, true);
    
    return RETS_SUCCESS;
    
}

uint8_t antiderivative(expression** result, expression* source, expression* variable, bool persistent) {
    
    uint8_t i;
    expression* temp_source = copy_expression(source);
    
    if (variable == NULL) {
        variable = guess_symbol(source, "", 0);
        if (variable == NULL) return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    }
    
    if (count_occurrences(source, variable, true) == 0) {
        *result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                 copy_expression(source),
                                 copy_expression(variable));
        simplify(*result, true);
        return RETS_SUCCESS;
    }
    
    if (temp_source->identifier == EXPI_ADDITION) {
        for (i = 0; i < temp_source->child_count; i++) {
            ERROR_CHECK(risch_integrate(temp_source->children[i], variable));
        }
    } else {
        ERROR_CHECK(risch_integrate(temp_source, variable));
    }
    
    simplify(temp_source, true);
    *result = temp_source;
    
    return RETS_SUCCESS;
    
}

uint8_t definite_integral(expression** result, expression* source, expression* variable, expression* lower_bound, expression* upper_bound) {
    
    expression* temp_source;
    expression* upper_bound_value;
    expression* lower_bound_value;
    
    if (variable == NULL) {
        variable = guess_symbol(source, "", 0);
    }
    
    antiderivative(&temp_source, source, variable, true);
    
    upper_bound_value = copy_expression(temp_source);
    lower_bound_value = copy_expression(temp_source);
    
    replace_occurences(upper_bound_value, variable, upper_bound);
    replace_occurences(lower_bound_value, variable, lower_bound);
    
    *result = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                             upper_bound_value,
                             lower_bound_value);
    
    ERROR_CHECK(simplify(*result, true));
    
    return RETS_SUCCESS;
    
}

