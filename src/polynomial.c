
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

return_status expression_to_sparse_polynomial(expression* source, const expression* variable);
void sparse_polynomial_to_expression(expression* source);
return_status sparse_polynomial_to_dense_polynomial(expression* source);
void dense_polynomial_to_sparse_polynomial(expression* source);

void any_expression_to_expression(expression* source) {
    if (source->identifier == EXPI_POLYNOMIAL_SPARSE) {
        sparse_polynomial_to_expression(source);
    } else if (source->identifier == EXPI_POLYNOMIAL_DENSE) {
        dense_polynomial_to_sparse_polynomial(source);
        sparse_polynomial_to_expression(source);
    } else {
        return;
    }
}

void any_expression_to_expression_recursive(expression* source) {
    uint8_t i;
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        any_expression_to_expression_recursive(source->children[i]);
    }
    any_expression_to_expression(source);
}

return_status any_expression_to_sparse_polynomial(expression* source, const expression* variable) {
    if (source->identifier == EXPI_POLYNOMIAL_SPARSE) {
        sort_sparse_polynomial(source);
        return RETS_SUCCESS;
    } else if (source->identifier == EXPI_POLYNOMIAL_DENSE) {
        dense_polynomial_to_sparse_polynomial(source);
    } else {
        ERROR_CHECK(expression_to_sparse_polynomial(source, variable));
    }
    
    return RETS_SUCCESS;
    
}

return_status any_expression_to_dense_polynomial(expression* source, const expression* variable) {
    
    if (source->identifier == EXPI_POLYNOMIAL_SPARSE) {
        ERROR_CHECK(sparse_polynomial_to_dense_polynomial(source));
    } else if (source->identifier == EXPI_POLYNOMIAL_DENSE) {
        return RETS_SUCCESS;
    } else {
        ERROR_CHECK(expression_to_sparse_polynomial(source, variable));
        ERROR_CHECK(sparse_polynomial_to_dense_polynomial(source));
    }
    
    return RETS_SUCCESS;
    
}

return_status validate_sparse_polynomial(expression* source, bool allow_decimal_exponents, bool allow_negative_exponents, bool allow_arbitrary_base) {
    
    uint8_t i;
    expression* temp_base = NULL;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->children[2] != NULL) {
            temp_base = copy_expression(source->children[i]->children[2]);
            break;
        }
    }
    
    if (temp_base == NULL) temp_base = new_symbol(EXPI_SYMBOL, "x");
    
    for (i = 0; i < source->child_count; i++) {
        
        if (source->children[i]->children[2] == NULL) {
            source->children[i]->children[2] = copy_expression(temp_base);
        }
        
        if (source->children[i]->children[0]->identifier == EXPI_LITERAL &&
            source->children[i]->children[0]->value.numeric.denominator != 1 &&
            !allow_decimal_exponents) {
            free_expression(temp_base, false);
            return RETS_ERROR;
        }
        
        if (source->children[i]->children[0]->identifier == EXPI_LITERAL &&
            source->children[i]->children[0]->sign == -1 &&
            !allow_negative_exponents) {
            free_expression(temp_base, false);
            return RETS_ERROR;
        }
        
        if (source->children[i]->children[2]->identifier != EXPI_SYMBOL && !allow_arbitrary_base) {
            free_expression(temp_base, false);
            return RETS_ERROR;
        }
        
        if (!expressions_are_identical(source->children[i]->children[2], source->children[0]->children[2], true)) {
            free_expression(temp_base, false);
            return RETS_ERROR;
        }
        
    }
    
    free_expression(temp_base, false);
    
    return RETS_SUCCESS;
    
}

void sort_sparse_polynomial(expression* source) {
    
    uint8_t i, j;
    uint8_t hightest_exponent_index = 0;
    double hightest_exponent_value;
    expression* result = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    
    for (i = 0; i < source->child_count; i++) {
        
        hightest_exponent_value = -1000000;
        
        for (j = 0; j < source->child_count; j++) {
            if (source->children[j] == NULL) continue;
            if (literal_to_double(source->children[j]->children[0]) >= hightest_exponent_value) {
                hightest_exponent_index = j;
                hightest_exponent_value = literal_to_double(source->children[j]->children[0]);
            }
        }
        
        append_child(result, source->children[hightest_exponent_index]);
        source->children[hightest_exponent_index] = NULL;
        
    }
    
    replace_expression(source, result);
    
}

void expression_to_sparse_polynomial_term(expression* source, const expression* variable) {
    
    uint8_t i;
    expression* result;
    
    if (count_occurrences(source, copy_expression(variable), false) == 0) {
        result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                new_literal(1, 0, 1),
                                copy_expression(source),
                                copy_expression(variable));
        replace_expression(source, result);
        return;
    }
    
    result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                            NULL,
                            NULL,
                            NULL);
    
    if (source->identifier == EXPI_MULTIPLICATION) {
        result->children[1] = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
        for (i = 0; i < source->child_count; i++) {
            if (source->children[i] == NULL) continue;
            if (count_occurrences(source->children[i], copy_expression(variable), false) == 0) {
                append_child(result->children[1], source->children[i]);
                source->children[i] = NULL;
            }
        }
        simplify(source, true);
        simplify(result->children[1], true);
    } else {
        result->children[1] = new_literal(1, 1, 1);
    }
    
    if (source->identifier == EXPI_EXPONENTATION) {
        result->children[0] = copy_expression(source->children[1]);
        result->children[2] = copy_expression(source->children[0]);
    } else {
        result->children[0] = new_literal(1, 1, 1);
        result->children[2] = copy_expression(source);
    }
    
    replace_expression(source, result);
    
}

return_status expression_to_sparse_polynomial(expression* source, const expression* variable) {
    
    uint8_t i;
    expression* temp_source = copy_expression(source);
    expression* temp_variable;
    expression* result;
    
    if (variable == NULL) {
        temp_variable = guess_symbol(source, "", 0);
        if (temp_variable == NULL) {
            temp_variable = new_symbol(EXPI_SYMBOL, "x");
        }
    } else {
        temp_variable = copy_expression(variable);
    }
    
    result = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    
    if (temp_source->identifier == EXPI_ADDITION) {
        for (i = 0; i < temp_source->child_count; i++) {
            if (temp_source->children[i] == NULL) continue;
            expression_to_sparse_polynomial_term(temp_source->children[i], temp_variable);
            append_child(result, copy_expression(temp_source->children[i]));
        }
    } else {
        simplify(temp_source, true);
        expression_to_sparse_polynomial_term(temp_source, temp_variable);
        append_child(result, copy_expression(temp_source));
    }
    
    free_expression(temp_source, false);
    
    if (validate_sparse_polynomial(result, true, true, true) == RETS_ERROR) {
        free_expression(result, false);
        free_expression(temp_variable, false);
        return RETS_ERROR;
    } else {
        sort_sparse_polynomial(result);
        replace_expression(source, result);
        free_expression(temp_variable, false);
        return RETS_SUCCESS;
    }
    
}

void sparse_polynomial_to_expression(expression* source) {
    
    uint8_t i;
    expression* result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count; i++) {
        append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                            copy_expression(source->children[i]->children[1]),
                                            new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                           copy_expression(source->children[i]->children[2]),
                                                           copy_expression(source->children[i]->children[0]))));
    }
    
    simplify(result, true);
    replace_expression(source, result);
    
}

return_status sparse_polynomial_to_dense_polynomial(expression* source) {
    
    uint8_t i;
    expression* result = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_DENSE, 2,
                                        copy_expression(source->children[0]->children[2]),
                                        new_expression(EXPT_STRUCTURE, EXPI_LIST, 0));
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->children[0]->identifier != EXPI_LITERAL &&
            source->children[i]->children[0]->value.numeric.denominator != 1) {
            free_expression(result, false);
            return RETS_ERROR;
        }
    }
    
    for (i = 0; i < source->children[0]->children[0]->value.numeric.numerator + 1; i++) {
        append_child(result->children[1], new_literal(1, 0, 1));
    }
    
    for (i = 0; i < source->child_count; i++) {
        replace_expression(result->children[1]->children[source->children[i]->children[0]->value.numeric.numerator], copy_expression(source->children[i]->children[1]));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

void dense_polynomial_to_sparse_polynomial(expression* source) {
    
    uint8_t i;
    expression* result = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    
    for (i = 0; i < source->children[1]->child_count; i++) {
        if (source->children[1]->children[i] == 0) continue;
        if (source->children[1]->children[i]->value.numeric.numerator != 0) {
            append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                                new_literal(1, i, 1),
                                                copy_expression(source->children[1]->children[i]),
                                                copy_expression(source->children[0])));
        }
    }
    
    sort_sparse_polynomial(result);
    replace_expression(source, result);
    
}

void quadratic_formula(expression** result, expression* a, expression* b, expression* c) {
    
    expression* discriminant;
    expression* divisor;
    
    discriminant = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                 copy_expression(b),
                                                 new_literal(1, 2, 1)),
                                  new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                                 new_literal(-1, 4, 1),
                                                 copy_expression(a),
                                                 copy_expression(c)));
    
    simplify(discriminant, true);
    
    divisor = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                             new_literal(1, 2, 1),
                             copy_expression(a));
    
    simplify(discriminant, true);
    
    if (discriminant->identifier == EXPI_LITERAL && discriminant->value.numeric.numerator == 0) {
        
        *result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 1,
                                 new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                               new_literal(-1, 1, 1),
                                                               copy_expression(b)),
                                                copy_expression(divisor)));
        
        simplify(*result, true);
        
    } else {
        
        discriminant = new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                      discriminant,
                                      new_literal(1, 1, 2));
        
        simplify(discriminant, true);
        
        (*result) = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                   new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                                 new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                                copy_expression(b),
                                                                                new_literal(-1, 1, 1)),
                                                                 copy_expression(discriminant)),
                                                  copy_expression(divisor)),
                                   new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                                                 new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                                copy_expression(b),
                                                                                new_literal(-1, 1, 1)),
                                                                 copy_expression(discriminant)),
                                                  copy_expression(divisor)));
        
        simplify(*result, true);
        
    }
    
}

return_status polysolve_quadratic(expression* source) {
    
    uint8_t i;
    expression* temp_source = copy_expression(source->children[0]);
    expression* temp;
    expression* result;
    
    if (temp_source->child_count > 3) {
        return RETS_ERROR;
    }
    
    if (temp_source->child_count == 2) {
        append_child(temp_source, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                                 new_literal(1, 0, 1),
                                                 new_literal(1, 0, 1),
                                                 copy_expression(temp_source->children[0]->children[2])));
    }
    
    temp = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                          copy_expression(temp_source->children[0]->children[0]),
                          copy_expression(temp_source->children[1]->children[0]));
    
    simplify(temp, true);
    
    if (temp->sign != 1) return RETS_ERROR;
    if (literal_to_double(temp) != 2) return RETS_ERROR;
    
    free_expression(temp, true);
    
    quadratic_formula(&result, temp_source->children[0]->children[1], temp_source->children[1]->children[1], temp_source->children[2]->children[1]);
    
    temp = new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                          copy_expression(temp_source->children[1]->children[2]),
                          copy_expression(temp_source->children[1]->children[0]));
    
    simplify(temp, true);
    
    for (i = 0; i < result->child_count; i++) {
        
        replace_expression(result->children[i], new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                               copy_expression(temp),
                                                               copy_expression(result->children[i])));
        
        if (temp->identifier != EXPI_SYMBOL) {
            solve(result->children[i], NULL);
        }
        
    }
    
    free_expression(temp_source, true);
    free_expression(temp, true);
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

return_status polysolve(expression* source, expression* variable) {
    
    expression* temp_source;
    
    if (variable == NULL) {
        variable = guess_symbol(source, "", 0);
    }
    
    subtract_rhs(source);
    temp_source = copy_expression(source);
    
    if (expression_to_sparse_polynomial(temp_source->children[0], variable) == RETS_ERROR) {
        free_expression(temp_source, false);
        return RETS_ERROR;
    }
    
    if (polysolve_quadratic(temp_source) == RETS_ERROR) {
        free_expression(temp_source, false);
        return RETS_ERROR;
    }
    
    replace_expression(source, temp_source);
    
    return RETS_ERROR;
    
}

uint8_t poly_div(expression** quotient, expression** remainder, const expression* a, const expression* b, int8_t degree) {
    
    expression* symbol;
    expression* a_temp = copy_expression(a);
    expression* b_temp = copy_expression(b);
    uint8_t a_degree;
    uint8_t b_degree;
    uint8_t power;
    expression* coefficient;
    expression* monomial;
    expression* result;
    expression* temp_quotient;
    expression* temp_remainder;
    
    symbol = get_symbol(a);
    
    simplify(a_temp, true);
    simplify(b_temp, true);
    
    if (any_expression_to_sparse_polynomial(a_temp, symbol) == RETS_ERROR ||
        any_expression_to_sparse_polynomial(b_temp, symbol) == RETS_ERROR ||
        validate_sparse_polynomial(a_temp, false, false, true) == RETS_ERROR ||
        validate_sparse_polynomial(b_temp, false, false, false) == RETS_ERROR) {
        free_expressions(3, symbol, a_temp, b_temp);
        return RETS_ERROR;
    }
    
    a_degree = (degree == -1) ? a_temp->children[0]->children[0]->value.numeric.numerator : degree;
    b_degree = b_temp->children[0]->children[0]->value.numeric.numerator;
    
    if (a_degree < b_degree) {
        if (quotient != NULL) *quotient = new_literal(1, 0, 1);
        if (remainder != NULL) *remainder = copy_expression(a_temp);
        free_expressions(3, symbol, a_temp, b_temp);
        return RETS_SUCCESS;
    }
    
    power = a_degree - b_degree;
    
    coefficient = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                 (a_temp->children[0]->children[0]->value.numeric.numerator == a_degree) ? copy_expression(a_temp->children[0]->children[1]) : new_literal(1, 0, 1),
                                 copy_expression(b_temp->children[0]->children[1]));
    simplify(coefficient, true);
    
    monomial = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 1,
                              new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                             new_literal(1, power, 1),
                                             coefficient,
                                             copy_expression(symbol)));
    
    result = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                            copy_expression(a),
                            new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                           copy_expression(monomial),
                                           copy_expression(b)));
    simplify(result, true);
    
    if (any_expression_to_sparse_polynomial(result, symbol) == RETS_ERROR) {
        free_expressions(5, symbol, a_temp, b_temp, monomial, result);
        return RETS_ERROR;
    }
    
    if (expressions_are_equivalent(result, new_literal(1, 0, 1), false)) {
        
        if (quotient != NULL) *quotient = copy_expression(monomial);
        if (remainder != NULL) *remainder = new_literal(1, 0, 1);
        
    } else {
        
        poly_div(&temp_quotient, &temp_remainder, result, b_temp, a_degree - 1);
        
        replace_expression(temp_quotient, new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                         copy_expression(temp_quotient),
                                                         copy_expression(monomial)));
        simplify(temp_quotient, true);
        
        if (any_expression_to_sparse_polynomial(temp_quotient, symbol) == RETS_ERROR ||
            any_expression_to_sparse_polynomial(temp_remainder, symbol) == RETS_ERROR) {
            return RETS_ERROR;
        }
        
        if (quotient != NULL) *quotient = copy_expression(temp_quotient);
        if (remainder != NULL) *remainder = copy_expression(temp_remainder);
        free_expressions(2, temp_quotient, temp_remainder);
        
    }
    
    free_expressions(5, symbol, a_temp, b_temp, monomial, result);
    
    return RETS_SUCCESS;
    
}

bool poly_is_square_free(expression* source) {
    
    expression* gcd;
    expression* temp_source = copy_expression(source);
    
    simplify(temp_source, true);
    any_expression_to_sparse_polynomial(temp_source, NULL);
    
    poly_log_gcd(&gcd, temp_source);
    
    free_expression(temp_source, true);
    
    if (expressions_are_equivalent(gcd, new_literal(1, 1, 1), false)) {
        return true;
    } else {
        return false;
    }
    
}

void make_monic(expression* source) {
    
    uint8_t i;
    expression* result;
    
    any_expression_to_sparse_polynomial(source, NULL);
    
    result = new_expression(EXPT_STRUCTURE, EXPI_POLYNOMIAL_SPARSE, 0);
    
    for (i = 0; i < source->child_count; i++) {
        append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                            copy_expression(source->children[i]->children[0]),
                                            new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                           copy_expression(source->children[i]->children[1]),
                                                           copy_expression(source->children[0]->children[1])),
                                            copy_expression(source->children[i]->children[2])));
    }
    
    simplify(result, true);
    replace_expression(source, result);
    
}

uint8_t poly_gcd(expression** gcd, const expression* a, const expression* b) {
    
    expression* symbol;
    expression* a_temp;
    expression* b_temp;
    expression* temp;
    expression* quotient;
    expression* remainder;
    
    if (expressions_are_equivalent(a, new_literal(1, 0, 1), false)) {
        *gcd = copy_expression(b);
        return RETS_SUCCESS;
    }
    
    if (expressions_are_equivalent(b, new_literal(1, 0, 1), false)) {
        *gcd = copy_expression(a);
        return RETS_SUCCESS;
    }
    
    a_temp = copy_expression(a);
    b_temp = copy_expression(b);
    
    simplify(a_temp, true);
    simplify(b_temp, true);
    
    symbol = get_symbol(a_temp);
    
    if (any_expression_to_sparse_polynomial(a_temp, symbol) == RETS_ERROR ||
        any_expression_to_sparse_polynomial(b_temp, symbol) == RETS_ERROR ||
        validate_sparse_polynomial(a_temp, false, false, true) == RETS_ERROR ||
        validate_sparse_polynomial(b_temp, false, false, false) == RETS_ERROR) {
        free_expressions(3, symbol, a_temp, b_temp);
        return RETS_ERROR;
    }
    
    sort_sparse_polynomial(a_temp);
    sort_sparse_polynomial(b_temp);
    
    if (expression_is_greater_than(b_temp->children[0]->children[0], a_temp->children[0]->children[0], true)) {
        temp = a_temp;
        a_temp = b_temp;
        b_temp = temp;
    }
    
    poly_div(&quotient, &remainder, a_temp, b_temp, -1);
    
    if (expressions_are_equivalent(remainder, new_literal(1, 0, 1), false)) {
        *gcd = copy_expression(b_temp);
    } else {
        poly_gcd(gcd, b_temp, remainder);
    }
    
    free_expressions(5, symbol, a_temp, b_temp, quotient, remainder);
    
    make_monic(*gcd);
    
    return RETS_SUCCESS;
    
}

void poly_log_gcd(expression** gcd, const expression* source) {
    
    expression* symbol = get_symbol(source);
    expression* a = copy_expression(source);
    expression* a_derivative;
    
    any_expression_to_sparse_polynomial(a, symbol);
    derivative(&a_derivative, copy_expression(a), copy_expression(symbol), false);
    
    any_expression_to_sparse_polynomial(a_derivative, symbol);
    
    poly_gcd(gcd, a, a_derivative);
    
    free_expressions(3, symbol, a, a_derivative);
    
}

uint8_t factor_square_free(expression** factors, const expression* source) {
    
    uint8_t i = 1;
    expression* symbol = get_symbol(source);
    expression* a = copy_expression(source);
    expression* b = NULL;
    expression* c = NULL;
    expression* g = NULL;
    expression* w = NULL;
    expression* y = NULL;
    expression* z = NULL;
    expression* temp;
    
    *factors = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    derivative(&b, a, symbol, true);
    poly_log_gcd(&c, a);
    
    if (expressions_are_equivalent(c, new_literal(1, 1, 1), false)) {
        
        w = copy_expression(a);
        
    } else {
        
        poly_div(&w, NULL, a, c, -1);
        poly_div(&y, NULL, b, c, -1);
        
        derivative(&temp, w, symbol, true);
        z = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                           copy_expression(y),
                           temp);
        simplify(z, true);
        
        while (!expressions_are_equivalent(z, new_literal(1, 0, 1), false)) {
            
            free_expression(g, false);
            poly_gcd(&g, w, z);
            
            if (!expressions_are_equivalent(g, new_literal(1, 1, 1), false)) {
                append_child(*factors, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                                      copy_expression(g),
                                                      new_literal(1, i, 1)));
            }
            
            temp = copy_expression(w);
            free_expression(w, false);
            poly_div(&w, NULL, temp, g, -1);
            free_expression(temp, false);
            
            free_expression(y, false);
            poly_div(&y, NULL, z, g, -1);

            derivative(&temp, w, symbol, true);
            free_expression(z, false);
            z = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                               copy_expression(y),
                               copy_expression(temp));
            free_expression(temp, false);
            simplify(z, true);
            
            i++;
            
        }
        
    }
    
    append_child(*factors, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                          copy_expression(w),
                                          new_literal(1, i, 1)));
    
    free_expressions(8, symbol, a, b, c, g, w, y, z);
    
    return RETS_SUCCESS;
    
}
