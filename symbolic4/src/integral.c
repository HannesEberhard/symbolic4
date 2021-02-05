
/*
 
 Copyright (c) 2020 Hannes Eberhard
 
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

#include "includes.h"

bool expression_is_risch_integrable(expression* source, expression* variable) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (!expression_is_risch_integrable(source->children[i], variable)) return false;
    }
    
    if (source->identifier == EXPI_EXPONENTIATION && source->children[0]->value.symbolic[0] != 'e' && count_occurrences(source->children[1], variable, true) != 0) {
        return false;
    } else if (source->identifier == EXPI_EXPONENTIATION && count_occurrences(source->children[0], variable, true) != 0 && source->children[1]->identifier != EXPI_LITERAL) {
        return false;
    }  else if (source->identifier == EXPI_EXPONENTIATION && count_occurrences(source->children[0], variable, true) != 0 && source->children[1]->value.numeric.denominator != 1) {
        return false;
    } else if (source->identifier == EXPI_LOG) {
        return false;
    } else {
        return true;
    }
    
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
    
    if (source->identifier == EXPI_EXPONENTIATION && source->children[0]->value.symbolic[0] == 'e') {
        extension = new_expression(EXPT_STRUCTURE, EXPI_EXTENSION, 2,
                                   new_symbol("EE____"),
                                   copy_expression(source->children[1]));
    } else if (source->identifier == EXPI_LN) {
        extension = new_expression(EXPT_STRUCTURE, EXPI_EXTENSION, 2,
                                   new_symbol("LE____"),
                                   copy_expression(source->children[1]));
    } else {
        return;
    }
    
    utoa(extension->children[0]->value.symbolic + 2, extensions->child_count);
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
        
        if (expressions_are_identical(remainder, new_literal(1, 0, 1), false)) {
            *rational_part = NULL;
        } else {
            *rational_part = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                            remainder,
                                            temp->children[1]);
        }
        
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
    expression* variable = get_symbol(source->children[0]);
    expression* z = new_symbol("ZE");
    expression* a = copy_expression(source->children[0]);
    expression* b = copy_expression(source->children[1]);
    expression* b_derivative;
    expression* resultant_primitive_part;
    expression* factors;
    expression* factor;
    expression* c;
    expression* v;
    expression* temp;
    expression* result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    any_expression_to_sparse_polynomial(a, variable);
    any_expression_to_sparse_polynomial(b, variable);
    
    derivative(&b_derivative, b, variable, true);
    
    temp = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                          copy_expression(a),
                          new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                         copy_expression(z),
                                         copy_expression(b_derivative)));
    simplify(temp, true);
    
    resultant(&resultant_primitive_part, temp, b, variable);
    free_expression(temp, false);
    any_expression_to_sparse_polynomial(resultant_primitive_part, z);
    make_monic(resultant_primitive_part);
    
    factor_square_free(&factors, resultant_primitive_part);
    
    for (i = 0; i < factors->child_count; i++) {
        
        factor = factors->children[i];
        
        c = new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                           new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
                                          copy_expression(factor->children[0]),
                                          copy_expression(factor->children[1])),
                           new_literal(1, 0, 1));
        simplify(c, true);
        solve(c, z);
        embed_in_list(c);
        
        if (factor->children[1]->value.numeric.numerator <= 2) {
            
            for (j = 0; j < c->child_count; j++) {

                temp = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                      copy_expression(a),
                                      new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                     copy_expression(c->children[j]->children[1]),
                                                     copy_expression(b_derivative)));
                simplify(temp, true);
                any_expression_to_sparse_polynomial(temp, variable);
                poly_gcd(&v, temp, b);
                make_monic(v);
                
                append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                    copy_expression(c->children[j]->children[1]),
                                                    new_expression(EXPT_FUNCTION, EXPI_LN, 1,
                                                                   copy_expression(v))));
                
                free_expressions(2, temp, v);
                
            }
            
        } else {
            
            printf("not implemented");
            
        }
        
        free_expression(c, false);
        
    }
    
    simplify(result, true);
    free_expressions(7, variable, z, a, b, b_derivative, resultant_primitive_part, factors);
    replace_expression(source, result);
    
}

void hermite_reduction(expression* source) {
    
    uint8_t i;
    expression* quotient;
    expression* remainder;
    expression* d;
    expression* b;
    uint8_t m;
    uint8_t n;
    expression* a = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    expression* c = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    
    poly_div(&quotient, &remainder, source->children[0], source->children[1], -1);
    poly_log_gcd(&d, source->children[1]);
    poly_div(&b, NULL, source->children[1], d, -1);
    
    m = get_degree(b);
    n = get_degree(d);
    
//    for (i = 0; i < m - 1; i++) {
//        append_child(c, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
//                                       ))
//    }
    
    print_expression(quotient);
    print_expression(remainder);
    print_expression(d);
    print_expression(b);
    
    
    
    
}

uint8_t risch_integrate_rational_part(expression* source) {
    
    if (source == NULL) return RETS_UNCHANGED;
    
    if (poly_is_square_free(source->children[1])) {
        rothstein_trager_method(source);
    } else {
        hermite_reduction(source);
    }
    
    return RETS_CHANGED;
    
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
        if (variable == NULL) return RETS_ERROR; //set_errorxx(ERRD_SYNTAX, ERRI_ARGUMENTS, "", false);
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
    
    replace_occurrences(upper_bound_value, variable, upper_bound);
    replace_occurrences(lower_bound_value, variable, lower_bound);
    
    *result = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                             upper_bound_value,
                             lower_bound_value);
    
    ERROR_CHECK(simplify(*result, true));
    
    return RETS_SUCCESS;
    
}

