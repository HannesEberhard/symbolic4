
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

return_status simplify_literal(expression* source);

return_status simplify_addition_literal_literal(expression* a, expression* b);
void simplify_addition_floating_literal(expression* a, expression* b);
void simplify_addition_floating_floating(expression* a, expression* b);
return_status simplify_addition_equation_list__value_function_structure(expression* a, expression* b);
return_status simplify_addition_equation_equation(expression* a, expression* b);
return_status simplify_addition_log_log(expression* a, expression* b);
void simplify_addition_list_list(expression* a, expression* b);
return_status evaluate_addition(expression* source);
return_status simplify_addition(expression* source);

return_status simplify_literal(expression* source) {
    
    uintmax_t divisor;
    
    if (source->value.numeric.denominator == 0) {
        return set_error(ERRI_LITERAL_WITH_DENOMINATOR_ZERO, false, source->value.numeric.numerator);
    }
    
    divisor = gcd(source->value.numeric.numerator, source->value.numeric.denominator);
    source->value.numeric.numerator /= divisor;
    source->value.numeric.denominator /= divisor;
    
    if (source->sign > 0 || source->value.numeric.numerator == 0) {
        source->sign = 1;
    } else {
        source->sign = -1;
    }
    
    return RETS_SUCCESS;
    
}

return_status simplify_addition_literal_literal(expression* a, expression* b) {
    
    int8_t sign;
    uintmax_t a_numerator;
    uintmax_t b_numerator;
    uintmax_t numerator;
    uintmax_t denominator;
    
    numeric_value a_value = a->value.numeric;
    numeric_value b_value = b->value.numeric;
    
    if (multiplication(&a_numerator, a_value.numerator, b_value.denominator) == RETS_ERROR ||
        multiplication(&b_numerator, b_value.numerator, a_value.denominator) == RETS_ERROR ||
        multiplication(&denominator, a_value.denominator, b_value.denominator) == RETS_ERROR) {
        return RETS_UNCHANGED;
    }
    
    if (a->sign * b->sign == 1) {
        if (addition(&numerator, a_numerator, b_numerator) == RETS_ERROR) {
            return RETS_UNCHANGED;
        } else {
            sign = a->sign;
        }
    } else {
        if (a_numerator > b_numerator) {
            numerator = a_numerator - b_numerator;
            sign = a->sign;
        } else {
            numerator = b_numerator - a_numerator;
            sign = b->sign;
        }
    }
    
    replace_expression(a, new_literal(sign, numerator, denominator));
    free_expressions(1, b);
    
    if (simplify_literal(a) == RETS_ERROR) {
        return RETS_ERROR;
    } else {
        return RETS_CHANGED;
    }
    
}

void simplify_addition_floating_literal(expression* a, expression* b) {
    a->value.floating += n_evaluate(b, 0);
    free_expressions(1, b);
}

void simplify_addition_floating_floating(expression* a, expression* b) {
    a->value.floating += b->value.floating;
    free_expressions(1, b);
}

return_status simplify_addition_equation_list__value_function_structure(expression* a, expression* b) {
    
    uint8_t i;
    expression* temp;
    
    for (i = 0; i < a->child_count; i++) {
        
        temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                              copy_expression(b),
                              a->children[i]);
        
        if (simplify(temp, false) == RETS_ERROR) {
            return RETS_ERROR;
        }
        
        a->children[i] = temp;
        
    }
    
    free_expressions(1, b);
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_equation_equation(expression* a, expression* b) {
    
    uint8_t i;
    expression* temp;
    
    for (i = 0; i < 2; i++) {
        
        temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                              a->children[i],
                              b->children[i]);
        
        if (simplify(temp, false) == RETS_ERROR) {
            return RETS_ERROR;
        }
        
        a->children[i] = temp;
        
    }
    
    free_expressionx(b, false, true);
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_log_log(expression* a, expression* b) {
    
    if (!expressions_are_equal(a->children[1], b->children[1], true)) {
        return RETS_UNCHANGED;
    }
    
    expression* temp = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                      a->children[0],
                                      b->children[0]);
    
    if (simplify(temp, false) == RETS_ERROR) {
        free_expressionx(temp, false, true);
        return RETS_ERROR;
    }
    
    a->children[0] = temp;
    
    free_expressionx(b->children[1], false, false);
    free_expressionx(b, false, true);
    
    return RETS_CHANGED;
    
}

void simplify_addition_list_list(expression* a, expression* b) {
    
    uint8_t i;

#ifndef DEBUG
    a->children = smart_realloc(a->children, a->child_count + b->child_count, sizeof(expression*));
#endif

    for (i = 0; i < b->child_count; i++) {
        a->children[a->child_count + i] = b->children[i];
    }

    a->child_count += b->child_count;

    free_expressionx(b, false, true);
    
}

return_status simplify_addition_vector_vector(expression* a, expression* b) {
    
    uint8_t i;
    expression* temp;
    
    if (a->child_count < b->child_count) {
        swap_expressions(a, b);
    }
    
    for (i = 0; i < b->child_count; i++) {
        
        temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                              a->children[i],
                              b->children[i]);
        
        if (simplify(temp, false) == RETS_ERROR) {
            free_expressionx(temp, false, true);
            return RETS_ERROR;
        }
        
        a->children[i] = temp;
        
    }
    
    free_expressionx(b, false, true);
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_matrix_matrix(expression* a, expression* b) {
    
    uint8_t i;
    uint8_t j;
    expression* temp;
    
    if (a->child_count != b->child_count || a->child_count == 0 || a->children[0]->child_count != b->children[0]->child_count) {
        return RETS_UNCHANGED;
    }
    
    for (i = 0; i < a->child_count; i++) {
        
        for (j = 0; j < a->children[0]->child_count; j++) {
            
            temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                  a->children[i]->children[j],
                                  b->children[i]->children[j]);
            
            if (simplify(temp, false) == RETS_ERROR) {
                free_expressionx(temp, false, true);
                return RETS_ERROR;
            }
            
            a->children[i]->children[j] = temp;
            
        }
        
        free_expressionx(b->children[i], false, true);
        
    }
    
    free_expressionx(b, false, true);
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_polynomial_polynomial(expression* a, expression* b) {
    
    if (!expressions_are_equal(a->children[1], b->children[1], true)) {
        return RETS_UNCHANGED;
    }
    
    if (simplify_addition_vector_vector(a->children[0], b->children[0]) == RETS_ERROR) {
        return RETS_ERROR;
    }
    
    free_expressionx(b->children[1], false, false);
    free_expressionx(b, false, true);
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_polynomial__value_function_structure(expression* a, expression* b) {
    
    expression* temp;
    
    if (expression_to_polynomial(b, a->children[1]) == RETS_UNCHANGED) {
        return RETS_UNCHANGED;
    }
    
    temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                          a->children[0]->children[0],
                          b);
    
    if (simplify(temp, false) == RETS_ERROR) {
        free_expressionx(temp, false, true);
        return RETS_ERROR;
    }
    
    a->children[0]->children[0] = temp;
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_complex_complex(expression* a, expression* b) {
    
    expression* real_sum = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                          a->children[0],
                                          b->children[0]);
    expression* imaginary_sum = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                               a->children[1],
                                               b->children[1]);
    
    if (simplify(real_sum, false) == RETS_ERROR || simplify(imaginary_sum, false) == RETS_ERROR) {
        free_expressionx(real_sum, false, true);
        free_expressionx(imaginary_sum, false, true);
        return RETS_ERROR;
    }
    
    a->children[0] = real_sum;
    a->children[1] = imaginary_sum;
    
    free_expressionx(b, false, true);
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_complex__value_function_structure(expression* a, expression* b) {
    
    expression* real_sum = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                          a->children[0],
                                          b);
    
    if (simplify(real_sum, false) == RETS_ERROR) {
        free_expressionx(real_sum, false, true);
        return RETS_ERROR;
    }
    
    a->children[0] = real_sum;
    
    return RETS_CHANGED;
    
}

return_status simplify_addition_symbolic(expression* a, expression* b) {
    
    expression* a_factor;
    expression* b_factor;
    expression* a_term;
    expression* b_term;
    
    if (a->identifier == EXPI_MULTIPLICATION &&
        a->child_count > 1 &&
        a->children[0]->identifier == EXPI_LITERAL) {
        a_factor = copy_expression(a->children[0]);
        if (a->child_count == 2) {
            a_term = copy_expression(a->children[1]);
        } else {
            a_term = copy_expression(a);
            remove_child(a_term, 0);
        }
    } else {
        a_factor = new_literal(1, 1, 1);
        a_term = copy_expression(a);
    }
    
    if (b->identifier == EXPI_MULTIPLICATION &&
        b->child_count > 1 &&
        b->children[0]->identifier == EXPI_LITERAL) {
        b_factor = copy_expression(b->children[0]);
        if (b->child_count == 2) {
            b_term = copy_expression(b->children[1]);
        } else {
            b_term = copy_expression(b);
            remove_child(b_term, 0);
        }
    } else {
        b_factor = new_literal(1, 1, 1);
        b_term = copy_expression(b);
    }
    
    if (!expressions_are_equal(a_term, b_term, true)) {
        free_expressions(4, a_factor, b_factor, a_term, b_term);
        return RETS_UNCHANGED;
    }
    
    if (simplify_addition_literal_literal(a_factor, b_factor) == RETS_UNCHANGED) {
        free_expressions(4, a_factor, b_factor, a_term, b_term);
        return RETS_UNCHANGED;
    }
    
    replace_expression(a, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                         a_factor,
                                         a_term));
    
    free_expressions(2, b, b_term);
    
    return RETS_CHANGED;
    
}

return_status evaluate_addition(expression* source) {
    
    uint8_t i;
    uint8_t j;
    expression** a;
    expression** b;
    
    for (i = 0; i < source->child_count; i++) {
        if (are_equal(n_evaluate(source->children[i], 0), 0)) {
            free_expressions(1, source->children[i]);
            source->children[i] = NULL;
        }
    }
    
    for (i = 0; i < source->child_count; i++) {
        
        a = &source->children[i];
        if (*a == NULL) continue;
        
        for (j = 0; j < source->child_count; j++) {
            
            b = &source->children[j];
            if (*b == NULL || i == j) continue;
            
            if ((*a)->identifier == EXPI_LITERAL) {
                if ((*b)->identifier == EXPI_LITERAL) {
                    switch (simplify_addition_literal_literal(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_FLOATING) {
                if ((*b)->identifier == EXPI_LITERAL) {
                    simplify_addition_floating_literal(*a, *b);
                    *b = NULL;
                    continue;
                } else if ((*b)->identifier == EXPI_FLOATING) {
                    simplify_addition_floating_floating(*a, *b);
                    *b = NULL;
                    continue;
                }
            } else if ((*a)->identifier == EXPI_EQUATION) {
                if ((*b)->type == EXPT_VALUE || (*b)->type == EXPT_FUNCTION || (*b)->type == EXPT_STRUCTURE) {
                    switch (simplify_addition_equation_list__value_function_structure(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                } else if ((*b)->identifier == EXPI_EQUATION) {
                    switch (simplify_addition_equation_equation(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_LOG) {
                if ((*b)->identifier == EXPI_LOG) {
                    switch (simplify_addition_log_log(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_LIST) {
                if ((*b)->identifier == EXPI_LIST) {
                    simplify_addition_list_list(*a, *b);
                    *b = NULL;
                    continue;
                } else if ((*b)->type == EXPT_VALUE || (*b)->type == EXPT_FUNCTION || (*b)->type == EXPT_STRUCTURE) {
                    switch (simplify_addition_equation_list__value_function_structure(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_VECTOR) {
                if ((*b)->identifier == EXPI_VECTOR) {
                    switch (simplify_addition_vector_vector(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_MATRIX) {
                if ((*b)->identifier == EXPI_MATRIX) {
                    switch (simplify_addition_matrix_matrix(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_POLYNOMIAL) {
                if ((*b)->identifier == EXPI_POLYNOMIAL) {
                    switch (simplify_addition_polynomial_polynomial(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                } else if ((*b)->type == EXPT_VALUE || (*b)->type == EXPT_FUNCTION || (*b)->type == EXPT_STRUCTURE) {
                    switch (simplify_addition_polynomial__value_function_structure(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else if ((*a)->identifier == EXPI_COMPLEX) {
                if ((*b)->identifier == EXPI_COMPLEX) {
                    switch (simplify_addition_complex_complex(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                } else if ((*b)->type == EXPT_VALUE || (*b)->type == EXPT_FUNCTION || (*b)->type == EXPT_STRUCTURE) {
                    switch (simplify_addition_complex__value_function_structure(*a, *b)) {
                        case RETS_CHANGED: *b = NULL; continue;
                        case RETS_ERROR: return RETS_ERROR;
                        default: break;
                    }
                }
            } else {
                switch (simplify_addition_symbolic(*a, *b)) {
                    case RETS_CHANGED: *b = NULL; continue;
                    case RETS_ERROR: return RETS_ERROR;
                    default: break;
                }
            }
        }
        
    }
    
    remove_null_children(source);
    
    return RETS_SUCCESS;
    
}

return_status simplify_addition(expression* source) {
    
    flatten_structure(source, EXPI_ADDITION, -1);
    
    if (evaluate_addition(source) == RETS_ERROR) {
        return RETS_ERROR;
    }
    
    order_children(source, false);
    
    return RETS_SUCCESS;
    
}

return_status simplify(expression* source, bool recursive) {
    
    uint8_t i;
    return_status status = RETS_SUCCESS;
    
    if (source == NULL) {
        return RETS_SUCCESS;
    }
    
    for (i = 0; recursive && i < source->child_count; i++) {
        if (simplify(source->children[i], true) == RETS_ERROR) {
            return RETS_ERROR;
        }
    }
    
    switch (source->identifier) {
        case EXPI_LITERAL: status = simplify_literal(source); break;
        case EXPI_ADDITION: status = simplify_addition(source); break;
        case EXPI_SUBTRACTION: break;
        case EXPI_MULTIPLICATION: flatten_structure(source, EXPI_MULTIPLICATION, -1); break;
        case EXPI_DIVISION: break;
        case EXPI_EXPONENTIATION: break;
        case EXPI_ABS: break;
        case EXPI_LN: break;
        case EXPI_LOG: break;
        case EXPI_SIN: break;
        case EXPI_COS: break;
        case EXPI_TAN: break;
        case EXPI_ARCSIN: break;
        case EXPI_ARCCOS: break;
        case EXPI_ARCTAN: break;
        case EXPI_COMPLEX: break;
        default: status = RETS_SUCCESS;
    }
    
    return status;
    
}


//
//bool changed = false; ///< deprecated
//
//void simplify_literal(expression* source);
//
//void merge_additions_multiplications(expression* source);
//uint8_t numeric_addition(expression** result, expression* a, expression* b, bool persistent);
//uint8_t symbolic_addition(expression** result, expression* a, expression* b, bool persistent);
//void evaluate_addition(expression* source);
//void simplify_addition(expression* source);
//
//uint8_t numeric_multiplication(expression** result, expression* a, expression* b);
//uint8_t symbolic_multiplication(expression** result, expression* a, expression* b);
//void evaluate_multiplication(expression* source);
//uint8_t expand_multiplication(expression* source);
//void simplify_multiplication(expression* source);
//
//void simplify_literal(expression* source) {
//    uintmax_t divisor;
//    if (source->identifier != EXPI_LITERAL) return;
//    divisor = gcd(source->value.numeric.numerator, source->value.numeric.denominator);
//    source->value.numeric.numerator /= divisor;
//    source->value.numeric.denominator /= divisor;
//    if (literal_to_double(source) == 0) source->sign = 1;
//}
//
//void merge_additions_multiplications(expression* source) {
//
//    uint8_t i, j;
//    expression_identifier identifier;
//    expression* result;
//
//    identifier = source->identifier;
//
//    if (identifier != EXPI_ADDITION && identifier != EXPI_MULTIPLICATION) return;
//
//    for (i = 0; i < source->child_count; i++) {
//        if (source->children[i] == NULL) continue;
//        if (source->children[i]->identifier == identifier) break; /* source has at least one child with the same identifier -> break to continue the merge process */
//        if (i == source->child_count - 1) return; /* all children of source were searched an no one had the same identifier as the source -> expressions cannot be merged -> return */
//    }
//
//    result = new_expression(EXPT_OPERATION, identifier, 0);
//
//    for (i = 0; i < source->child_count; i++) {
//        if (source->children[i] == NULL) continue;
//        if (source->children[i]->identifier == identifier) {
//            merge_additions_multiplications(source->children[i]);
//            for (j = 0; j < source->children[i]->child_count; j++) {
//                append_child(result, copy_expression(source->children[i]->children[j]));
//            }
//        } else {
//            append_child(result, copy_expression(source->children[i]));
//        }
//    }
//
//    replace_expression(source, result);
//
//    changed = true;
//
//}
//
//uint8_t numeric_addition(expression** result, expression* a, expression* b, bool persistent) {
//
//    numeric_value a_value;
//    numeric_value b_value;
//    uintmax_t temp_1;
//    uintmax_t temp_2;
//    uintmax_t temp_3;
//
//    if (a->identifier != EXPI_LITERAL || b->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
//
//    a_value = a->value.numeric;
//    b_value = b->value.numeric;
//
//    if ((a->sign == 1 && b->sign == 1) || (a->sign == -1 && b->sign == -1)) {
//
//        if (multiplication(&temp_1, a_value.numerator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
//        if (multiplication(&temp_2, a_value.denominator, b_value.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//        if (addition(&temp_1, temp_1, temp_2) == RETS_ERROR) return RETS_UNCHANGED;
//
//        if (multiplication(&temp_2, a_value.denominator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
//
//        *result = new_literal(a->sign, temp_1, temp_2);
//
//    } else {
//
//        if (multiplication(&temp_1, a_value.numerator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
//        if (multiplication(&temp_2, a_value.denominator, b_value.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//        if (multiplication(&temp_3, a_value.denominator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
//
//        if (temp_1 > temp_2) {
//            *result = new_literal(a->sign, temp_1 - temp_2, temp_3);
//        } else {
//            *result = new_literal(b->sign, temp_2 - temp_1, temp_3);
//        }
//
//    }
//
//    simplify_literal(*result);
//
//    if (!persistent) {
//        free_expression(a, false);
//        free_expression(b, false);
//    }
//
//    changed = true;
//
//    return RETS_CHANGED;
//
//}
//
//uint8_t symbolic_addition(expression** result, expression* a, expression* b, bool persistent) {
//
//    expression* a_temp;
//    expression* b_temp;
//    expression* a_factor;
//    expression* b_factor;
//    expression* temp;
//
//    if (a->identifier == EXPI_MULTIPLICATION && a->children[0]->identifier == EXPI_LITERAL) {
//        a_factor = copy_expression(a->children[0]);
//        a_temp = copy_expression(a);
//        remove_child(a_temp, 0);
//        simplify(a_temp, false);
//    } else {
//        a_temp = copy_expression(a);
//        a_factor = new_literal(1, 1, 1);
//    }
//
//    if (b->identifier == EXPI_MULTIPLICATION && b->children[0]->identifier == EXPI_LITERAL) {
//        b_factor = copy_expression(b->children[0]);
//        b_temp = copy_expression(b);
//        remove_child(b_temp, 0);
//        simplify(b_temp, false);
//    } else {
//        b_temp = copy_expression(b);
//        b_factor = new_literal(1, 1, 1);
//    }
//
//    if (expressions_are_identical(a_temp, b_temp, true)) {
//        numeric_addition(&temp, a_factor, b_factor, false);
//        *result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                 temp,
//                                 a_temp);
//        free_expression(b_temp, false);
//        changed = true;
//        return RETS_CHANGED;
//    } else {
//        free_expression(a_temp, false);
//        free_expression(b_temp, false);
//        free_expression(a_factor, false);
//        free_expression(b_factor, false);
//        return RETS_UNCHANGED;
//    }
//
//}
//
//void evaluate_addition(expression* source) {
//
//    uint8_t i, j;
//    expression* temp_result;
//    expression* result;
//
//    for (i = 0; i < source->child_count; i++) {
//
//        if (source->children[i] == NULL) continue;
//
//        if (source->children[i]->identifier == EXPI_LITERAL && literal_to_double(source->children[i]) == 0) {
//            free_expression(source->children[i], false);
//            source->children[i] = NULL;
//        }
//
//        for (j = i + 1; j < source->child_count; j++) {
//
//            if (source->children[i] == NULL) continue;
//            if (source->children[j] == NULL) continue;
//
//            if (numeric_addition(&temp_result, source->children[i], source->children[j], true) == RETS_CHANGED) {
//                replace_expression(source->children[i], temp_result);
//                free_expression(source->children[j], false);
//                source->children[j] = NULL;
//                continue;
//            }
//
//            if (symbolic_addition(&temp_result, source->children[i], source->children[j], true) == RETS_CHANGED) {
//                replace_expression(source->children[i], temp_result);
//                free_expression(source->children[j], false);
//                source->children[j] = NULL;
//                continue;
//            }
//
//        }
//
//    }
//
//    remove_null_children(source);
//
//    switch (source->child_count) {
//        case 0: result = new_literal(1, 0, 1); break;
//        case 1: result = copy_expression(source->children[0]); break;
//        default: result = copy_expression(source); break;
//    }
//
//    replace_expression(source, result);
//
//}
//
//void simplify_addition(expression* source) {
//    merge_additions_multiplications(source);
//    evaluate_addition(source);
//    if (source->child_count > 0) order_children(source, false);
//}
//
//void simplify_subtraction(expression* source) {
//    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                              copy_expression(source->children[0]),
//                                              new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                             new_literal(-1, 1, 1),
//                                                             copy_expression(source->children[1]))));
//    changed = true;
//}
//
//uint8_t numeric_multiplication(expression** result, expression* a, expression* b) {
//
//    numeric_value a_value;
//    numeric_value b_value;
//    uintmax_t temp_1;
//    uintmax_t temp_2;
//
//    if (a->identifier != EXPI_LITERAL || b->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
//
//    a_value = a->value.numeric;
//    b_value = b->value.numeric;
//
//    if (multiplication(&temp_1, a_value.numerator, b_value.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//    if (multiplication(&temp_2, b_value.denominator, a_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
//
//    *result = new_literal(a->sign * b->sign, temp_1, temp_2);
//    simplify_literal(*result);
//
//    changed = true;
//
//    return RETS_CHANGED;
//
//}
//
//uint8_t symbolic_multiplication(expression** result, expression* a, expression* b) {
//
//    expression* a_temp;
//    expression* b_temp;
//    expression* a_exponent;
//    expression* b_exponent;
//    expression* temp;
//
//    if (a->identifier == EXPI_EXPONENTIATION && a->children[1]->identifier == EXPI_LITERAL) {
//        a_temp = copy_expression(a->children[0]);
//        a_exponent = copy_expression(a->children[1]);
//    } else {
//        a_temp = copy_expression(a);
//        a_exponent = new_literal(1, 1, 1);
//    }
//
//    if (b->identifier == EXPI_EXPONENTIATION && b->children[1]->identifier == EXPI_LITERAL) {
//        b_temp = copy_expression(b->children[0]);
//        b_exponent = copy_expression(b->children[1]);
//    } else {
//        b_temp = copy_expression(b);
//        b_exponent = new_literal(1, 1, 1);
//    }
//
//    if (expressions_are_identical(a_temp, b_temp, true)) {
//        numeric_addition(&temp, a_exponent, b_exponent, false);
//        *result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                 a_temp,
//                                 temp);
//        free_expression(b_temp, false);
//        changed = true;
//        return RETS_CHANGED;
//    } else {
//        free_expression(a_temp, false);
//        free_expression(b_temp, false);
//        free_expression(a_exponent, false);
//        free_expression(b_exponent, false);
//        return RETS_UNCHANGED;
//    }
//
//}
//
//uint8_t exponentiation_multiplication(expression** result, expression* a, expression* b) {
//
//    expression* a_result;
//    expression* b_result;
//
//    if (a->identifier != EXPI_EXPONENTIATION || b->identifier != EXPI_EXPONENTIATION || a->children[0]->identifier != EXPI_LITERAL || b->children[0]->identifier != EXPI_LITERAL || a->children[1]->identifier != EXPI_LITERAL || b->children[1]->identifier != EXPI_LITERAL) {
//        return RETS_UNCHANGED;
//    }
//
//    a_result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                               copy_expression(a->children[0]),
//                               new_literal(a->children[1]->sign, a->children[1]->value.numeric.numerator * b->children[1]->value.numeric.numerator, 1));
//
//    b_result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                               copy_expression(b->children[0]),
//                               new_literal(b->children[1]->sign, a->children[1]->value.numeric.numerator * b->children[1]->value.numeric.numerator, 1));
//
//    simplify(a_result, false);
//    simplify(b_result, false);
//
//    if (a_result->identifier != EXPI_EXPONENTIATION || b_result->identifier != EXPI_EXPONENTIATION) {
//        *result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                 new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                copy_expression(a_result),
//                                                copy_expression(b_result)),
//                                 new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                copy_expression(a->children[1]),
//                                                copy_expression(b->children[1])));
//        simplify(*result, true);
//        free_expressions(2, a_result, b_result);
//        changed = true;
//        return RETS_CHANGED;
//    } else {
//        free_expressions(2, a_result, b_result);
//        return RETS_UNCHANGED;
//    }
//
//}
//
//void evaluate_multiplication(expression* source) {
//
//    uint8_t i, j;
//    expression* temp_result;
//    expression* result;
//
//    for (i = 0; i < source->child_count; i++) {
//
//        if (source->children[i] == NULL) continue;
//
//        if (source->children[i]->identifier == EXPI_LITERAL && literal_to_double(source->children[i]) == 0) {
//            replace_expression(source, new_literal(1, 0, 1));
//            return;
//        }
//
//        if (source->children[i]->identifier == EXPI_LITERAL && literal_to_double(source->children[i]) == 1) {
//            free_expression(source->children[i], false);
//            source->children[i] = NULL;
//            continue;
//        }
//
//        for (j = i + 1; j < source->child_count; j++) {
//
//            if (source->children[i] == NULL) continue;
//            if (source->children[j] == NULL) continue;
//
//            if (numeric_multiplication(&temp_result, source->children[i], source->children[j]) == RETS_CHANGED) {
//                replace_expression(source->children[i], temp_result);
//                free_expression(source->children[j], false);
//                source->children[j] = NULL;
//                continue;
//            }
//
//            if (symbolic_multiplication(&temp_result, source->children[i], source->children[j]) == RETS_CHANGED) {
//                replace_expression(source->children[i], temp_result);
//                free_expression(source->children[j], false);
//                source->children[j] = NULL;
//                continue;
//            }
//
//            if (exponentiation_multiplication(&temp_result, source->children[i], source->children[j]) == RETS_CHANGED) {
//                replace_expression(source->children[i], temp_result);
//                free_expression(source->children[j], false);
//                source->children[j] = NULL;
//                continue;
//            }
//
//        }
//
//    }
//
//    remove_null_children(source);
//
//    switch (source->child_count) {
//        case 0: result = new_literal(1, 1, 1); break;
//        case 1: result = copy_expression(source->children[0]); break;
//        default: result = copy_expression(source); break;
//    }
//
//    replace_expression(source, result);
//
//}
//
//void expand_multiplication_addition_factors(expression* source) {
//
//    uint8_t i, j, k, l;
//    expression* result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
//
//    for (i = 0; i < source->child_count - 1; i++) {
//        for (j = 0; j < source->children[i]->child_count; j++) {
//            for (k = i + 1; k < source->child_count; k++) {
//                for (l = 0; l < source->children[k]->child_count; l++) {
//                    append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                        copy_expression(source->children[i]->children[j]),
//                                                        copy_expression(source->children[k]->children[l])));
//                }
//            }
//        }
//    }
//
//    simplify(result, true);
//    replace_expression(source, result);
//
//}
//
//uint8_t expand_multiplication(expression* source) {
//
//    uint8_t i;
//    expression* single_factors;
//    expression* addition_factors;
//    expression* result;
//
//    if (source->child_count == 0) return RETS_UNCHANGED;
//
//    for (i = 0; i < source->child_count; i++) {
//        if (source->children[i] == NULL) continue;
//        if (source->children[i]->identifier == EXPI_ADDITION) break; /* source has at least one child being an addition -> break to continue the expanding process */
//        if (i == source->child_count - 1) return RETS_UNCHANGED; /* all children of source were searched an no one had an addition as its child -> multiplication cannot be expanded -> return */
//    }
//
//    single_factors = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
//    addition_factors = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
//    result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
//
//    for (i = 0; i < source->child_count; i++) {
//
//        if (source->children[i] == NULL) continue;
//
//        if (source->children[i]->identifier == EXPI_ADDITION) {
//            append_child(addition_factors, copy_expression(source->children[i]));
//        } else {
//            append_child(single_factors, copy_expression(source->children[i]));
//        }
//
//    }
//
//    if (addition_factors->child_count > 1) {
//        expand_multiplication_addition_factors(addition_factors);
//    } else {
//        replace_expression(addition_factors, copy_expression(addition_factors->children[0]));
//    }
//
//    for (i = 0; i < addition_factors->child_count; i++) {
//        append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                            copy_expression(single_factors),
//                                            copy_expression(addition_factors->children[i])));
//    }
//
//    simplify(result, true);
//    replace_expression(source, result);
//
//    free_expression(single_factors, false);
//    free_expression(addition_factors, false);
//
//    changed = true;
//
//    return RETS_CHANGED;
//
//}
//
//void simplify_multiplication(expression* source) {
//    merge_additions_multiplications(source);
//    if (expand_multiplication(source) == RETS_CHANGED) return;
//    evaluate_multiplication(source);
//    order_children(source, false);
//}
//
//void simplify_division(expression* source) {
//
//    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                              copy_expression(source->children[0]),
//                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                             copy_expression(source->children[1]),
//                                                             new_literal(-1, 1, 1))));
//
//    changed = true;
//
//}
//
///**
//
// @brief Simplifies nested exponentiations
//
// @details
// This function applies the rule (a^m)^n = a^(m*a) where a, m and n may
// be arbitrary expressions. It must be called on the outer
// exponentiation.
//
// @warning
// - This function does not cope with possible changes in the domain of
// a simplified expression and thus incorrectly simplifies (x^2)^0.5 to
// x, for example.
//
// @param[in,out] source expression to be simplified
//
// @see
// - simplify_exponentiation
//
// */
//void merge_exponentiation_base_exponentiation(expression* source) {
//
//    expression* result;
//
//    if (source->identifier != EXPI_EXPONENTIATION || source->children[0]->identifier != EXPI_EXPONENTIATION) return;
//
//    result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                            copy_expression(source->children[0]->children[0]),
//                            new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                           copy_expression(source->children[0]->children[1]),
//                                           copy_expression(source->children[1])));
//    simplify(result->children[1], false);
//
//    replace_expression(source, result);
//    changed = true;
//
//}
//
///**
//
// @brief Removes identities from an exponentiation
//
// @details
// This function applies the following rules:
// x^0 = 1,
// x^1 = x,
// 0^x = 0, and
// 1^x = 1.
//
// @note
// - 0^0 is treated as 1
//
// @param[in,out] source exponentiation to be simplified
//
// @return
// - RETS_CHANGED if the source could be simplified. Rules about the
// simplification of exponentiations don't apply anymore as the
// identifier of the source has been changed
// - RETS_UNCHANGED if the source was left unchanged
//
// @see
// - simplify_exponentiation
//
// */
//return_status remove_exponentiation_identities(expression* source) {
//
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//
//    if (source->identifier != EXPI_EXPONENTIATION) return RETS_UNCHANGED;
//
//    if (exponent->identifier == EXPI_LITERAL && literal_to_double(exponent) == 0) {
//        replace_expression(source, new_literal(1, 1, 1));
//    } else if (exponent->identifier == EXPI_LITERAL && literal_to_double(exponent) == 1) {
//        replace_expression(source, copy_expression(base));
//    } else if (base->identifier == EXPI_LITERAL && literal_to_double(base) == 0) {
//        replace_expression(source, new_literal(1, 0, 1));
//    } else if (base->identifier == EXPI_LITERAL && literal_to_double(base) == 1) {
//        replace_expression(source, new_literal(1, 1, 1));
//    } else {
//        return RETS_UNCHANGED;
//    }
//
//    return RETS_CHANGED;
//
//}
//
///**
//
// @brief Simplifies exponentiations where the base is an addition and
// the exponent is a literal
//
// @details
// If the base has 2 children and the exponent is an integer <= 3, the
// expression will be expanded using binomial coefficients:
// (a+b)^2 = 2ab + a^2 + b^2.
// If the exponent is a fraction, an attempt is made to factorize the
// exponent and to cancel out the resulting exponents:
// (x^2+2*x+1)^0.5 = ((x+1)^2)^0.5 = 1+x.
//
// @warning
// - This function does not cope with possible changes in the domain of
// simplified expressions when exponents are cancelled.
//
// @param[in,out] source expression to be simplified
//
// @return
// - RETS_CHANGED if the source could be simplified. Rules about the
// simplification of exponentiations don't apply anymore as the
// identifier of the source has been changed
// - RETS_UNCHANGED if the source was left unchanged
//
// @see
// - simplify_exponentiation
//
// */
//return_status expand_exponentiation_base_addition(expression* source) {
//
//    uint8_t i;
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//    expression* result;
//    expression* factors;
//    uintmax_t* coefficients;
//
//    if (source->identifier != EXPI_EXPONENTIATION || base->identifier != EXPI_ADDITION || exponent->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
//
//    if (exponent->value.numeric.denominator != 1) {
//
////        TODO: Factor
//
//    } else if (base->child_count == 2 && exponent->value.numeric.numerator <= 3) {
//
//        result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
//        coefficients = binomial_coefficients(exponent->value.numeric.numerator);
//
//        for (i = 0; i <= exponent->value.numeric.numerator; i++) {
//            append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
//                                                new_literal(1, coefficients[i], 1),
//                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                               copy_expression(base->children[0]),
//                                                               new_literal(1, exponent->value.numeric.numerator - i, 1)),
//                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                               copy_expression(base->children[1]),
//                                                               new_literal(1, i, 1))));
//        }
//
//        smart_free(coefficients);
//        replace_expression(source, result);
//
//        changed = true;
//        return RETS_CHANGED;
//
//    }
//
//    return RETS_UNCHANGED;
//
//}
//
///**
//
// @brief Simplifies exponentiations where the base is a multiplication
//
// @details
// This function applies the rule (a*b)^c = a^c * b^c where a, b and c
// may be arbitrary expressions.
//
// @param[in,out] source exponentiation to simplify
//
// @return
// - RETS_CHANGED if the source could be simplified. Rules about the
// simplification of exponentiations don't apply anymore as the
// identifier of the source has been changed
// - RETS_UNCHANGED if the source was left unchanged
//
// @see
// - simplify_exponentiation
//
// */
//return_status expand_exponentiation_base_multiplication(expression* source) {
//
//    uint8_t i;
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//    expression* result;
//
//    if (source->identifier != EXPI_EXPONENTIATION || base->identifier != EXPI_MULTIPLICATION) return RETS_UNCHANGED;
//
//    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
//
//    for (i = 0; i < source->children[0]->child_count; i++) {
//        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                            copy_expression(base->children[i]),
//                                            copy_expression(exponent)));
//    }
//
//    replace_expression(source, result);
//
//    changed = true;
//    return RETS_CHANGED;
//
//}
//
///**
//
// @brief Simplifies an exponentiation where the exponent is an addition
//
// @details
// This function applies the rule a^(m+n) = a^m * a^n where a, m and n
// may be arbitrary expressions.
// @param[in,out] source exponentiation to be simplified
//
// @return
// - RETS_CHANGED if the source could be simplified. Rules about the
// simplification of exponentiations don't apply anymore as the
// identifier of the source has been changed
// - RETS_UNCHANGED if the source was left unchanged
//
// @see
// - simplify_exponentiation
//
// */
//return_status expand_exponentiation_exponent_addition(expression* source) {
//
//    uint8_t i;
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//    expression* result;
//
//    if (source->identifier != EXPI_EXPONENTIATION || exponent->identifier != EXPI_ADDITION) return RETS_UNCHANGED;
//
//    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
//
//    for (i = 0; i < exponent->child_count; i++) {
//        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                            copy_expression(base),
//                                            copy_expression(exponent->children[i])));
//    }
//
//    replace_expression(source, result);
//
//    changed = true;
//    return RETS_CHANGED;
//
//}
//
///**
//
// @brief Simplifies exponentiations with logarithms in the exponent
//
// @details
// This function applies the rule a^log(x, a) = x. If the exponent is a
// multiplication, its children are searched for matching logarithms. In
// that case, the rule a^(m*log(x, a)) = x^m is applied.
//
// @param[in,out] source exponentiation to be simplified
//
// @return
// - RETS_CHANGED if the source could be simplified. Rules about the
// simplification of exponentiations may not apply anymore as the
// identifier of the source has maybe been changed.
// - RETS_UNCHANGED if the source was left unchanged.
//
// @see
// - simplify_exponentiation
//
// */
//return_status simplify_exponentiation_exponent_logarithm(expression* source) {
//
//    uint8_t i;
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//
//    if (source->identifier != EXPI_EXPONENTIATION) return RETS_UNCHANGED;
//
//    if (exponent->identifier == EXPI_LOG && expressions_are_identical(base, exponent->children[1], true)) {
//        replace_expression(source, copy_expression(exponent->children[0]));
//        return RETS_CHANGED;
//    }
//
//    if (exponent->identifier == EXPI_MULTIPLICATION) {
//        for (i = 0; i < exponent->child_count; i++) {
//            if (exponent->children[i]->identifier == EXPI_LOG && expressions_are_identical(base, exponent->children[i]->children[1], true)) {
//                replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                          copy_expression(exponent->children[i]->children[0]),
//                                                          copy_expression(exponent)));
//                remove_child(source->children[1], i);
//                changed = true;
//                return RETS_CHANGED;
//            }
//        }
//    }
//
//    return RETS_UNCHANGED;
//
//}
//
//uint8_t numeric_exponentiationx(expression* source) {
//
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//    expression* base_result;
//    expression* factor;
//    expression* result;
//
//    uintmax_t temp;
//
//    if (base->identifier != EXPI_LITERAL || exponent->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
//
////    print_expression(base);
////    print_expression(exponent);
//
//    base_result = new_literal(1, 0, 1);
//    factor = new_literal(1, 1, 1);
//
//    simplify_literal(base);
//    simplify_literal(exponent);
//
////    if (literal_to_double(base) == -1 && exponent->value.numeric.denominator == 2) {
//
////        if ((exponent->value.numeric.numerator - 1) % 4 == 0) {
////            replace_expression(source, new_symbol("i"));
////        } else {
////            replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                      new_literal(-1, 1, 1),
////                                                      new_symbol("i")));
////        }
//
////        exponent->value.numeric.numerator %= exponent->value.numeric.denominator;
////
////        return RETS_CHANGED;
////
////    }
//
//    if (exponent->sign == -1) {
//        temp = base->value.numeric.numerator;
//        base->value.numeric.numerator = base->value.numeric.denominator;
//        base->value.numeric.denominator = temp;
//        exponent->sign = 1;
//    }
//
//    if (exponentiation(&base_result->value.numeric.numerator, base->value.numeric.numerator, exponent->value.numeric.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//    if (exponentiation(&base_result->value.numeric.denominator, base->value.numeric.denominator, exponent->value.numeric.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//
////    int_root(&factor->value.numeric.numerator, &base_result->value.numeric.numerator, base_result->value.numeric.numerator, exponent->value.numeric.denominator);
////    int_root(&factor->value.numeric.denominator, &base_result->value.numeric.denominator, base_result->value.numeric.denominator, exponent->value.numeric.denominator);
//
//    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
//
//    if (base->sign == -1 && exponent->value.numeric.numerator % 2 == 1) {
//        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                            new_literal(-1, 1, 1),
//                                            copy_expression(exponent)));
////        symbolic_exponentiation(result->children[result->child_count - 1]);
//    }
//
//    if (literal_to_double(base_result) == 1) {
//        append_child(result, copy_expression(factor));
//        simplify(result, true);
//        replace_expression(source, result);
//        free_expressions(2, base_result, factor);
//        return RETS_CHANGED;
//    } else if (literal_to_double(factor) == literal_to_double(base_result)) {
//        free_expressions(2, base_result, factor);
//        return RETS_UNCHANGED;
//    } else {
//        exponent->value.numeric.numerator = 1;
//        if (literal_to_double(factor) != 1) append_child(result, copy_expression(factor));
//        if (literal_to_double(base_result) != 1) append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                     copy_expression(base_result),
//                                                                                     copy_expression(exponent)));
//    }
//
//    simplify_multiplication(result);
//    replace_expression(source, result);
//
//    free_expressions(2, base_result, factor);
//
//    return RETS_CHANGED;
//
//}
//
///*uint8_t numeric_exponentiation(expression* source) {
//
//    expression* base = source->children[0];
//    expression* exponent = source->children[1];
//    expression* base_result;
//    expression* factor;
//    expression* result;
//
//    uintmax_t temp;
//
//    if (base->identifier != EXPI_LITERAL || exponent->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
//
//    base_result = new_literal(1, 0, 1);
//    factor = new_literal(1, 1, 1);
//
//    simplify_literal(base);
//    simplify_literal(exponent);
//
//    if (literal_to_double(base) == -1 && exponent->value.numeric.denominator == 2) {
//
//        if ((exponent->value.numeric.numerator - 1) % 4 == 0) {
//            replace_expression(source, new_symbol("i"));
//        } else {
//            replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                      new_literal(-1, 1, 1),
//                                                      new_symbol("i")));
//        }
//
//        return RETS_CHANGED;
//
//    }
//
//    if (exponent->sign == -1) {
//        temp = base->value.numeric.numerator;
//        base->value.numeric.numerator = base->value.numeric.denominator;
//        base->value.numeric.denominator = temp;
//        exponent->sign = 1;
//    }
//
//    if (exponentiation(&base_result->value.numeric.numerator, base->value.numeric.numerator, exponent->value.numeric.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//    if (exponentiation(&base_result->value.numeric.denominator, base->value.numeric.denominator, exponent->value.numeric.numerator) == RETS_ERROR) return RETS_UNCHANGED;
//
//    int_root(&factor->value.numeric.numerator, &base_result->value.numeric.numerator, base_result->value.numeric.numerator, exponent->value.numeric.denominator);
//    int_root(&factor->value.numeric.denominator, &base_result->value.numeric.denominator, base_result->value.numeric.denominator, exponent->value.numeric.denominator);
//
//    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
//
//    if (base->sign == -1 && exponent->value.numeric.numerator % 2 == 1) {
//        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                            new_literal(-1, 1, 1),
//                                            copy_expression(exponent)));
////        symbolic_exponentiation(result->children[result->child_count - 1]);
//    }
//
//    if (literal_to_double(factor) == literal_to_double(base_result)) {
//        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                            copy_expression(base_result),
//                                            new_literal(1, exponent->value.numeric.numerator + exponent->value.numeric.denominator, exponent->value.numeric.denominator)));
//        remove_exponentiation_identities(result->children[result->child_count - 1]);
//    } else {
//        if (literal_to_double(factor) != 1) append_child(result, copy_expression(factor));
//        if (literal_to_double(base_result) != 1) append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                     copy_expression(base_result),
//                                                                                     copy_expression(exponent)));
//    }
//
//    simplify_multiplication(result);
//    replace_expression(source, result);
//
//    free_expressions(2, base_result, factor);
//
//    return RETS_CHANGED;
//
//}*/
//
///**
//
// @brief Evaluates a numeric exponentiation
//
// @details
// As all numeric values are represented as a fraction, any numeric
// exponentiation is of the form (a/b)^(m/n) which is equivalent to
// a^(m/n) / b^(m/n).
//
// @param[in,out] source exponentiation to be evaluated
//
// @return
// - RETS_CHANGED if the expression could be evaluated or if it already
// was fully reduced
// - RETS_UNCHANGED if the power operation would cause an integer
// overflow
//
// @see
// - simplify_exponentiation
// - TODO: multiplication exponentiation
//
// */
//return_status numeric_exponentiation(expression* source) {
//
//    expression* base;
//    expression* exponent;
//    expression* base_result;
//    expression* factor;
//    int8_t sign = 1;
//    uintmax_t temp;
//
//    if (source->identifier != EXPI_EXPONENTIATION || source->children[0]->identifier != EXPI_LITERAL || source->children[1]->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
//
//    base = copy_expression(source->children[0]);
//    exponent = copy_expression(source->children[1]);
//
//    base_result = new_literal(1, 0, 1);
//    factor = new_literal(1, 1, 1);
//
//    simplify_literal(base);
//    simplify_literal(exponent);
//
//    if (base->sign == -1 && exponent->value.numeric.numerator % 2 == 1) {
//        sign = -1;
//    }
//
//    base->sign = 1;
//
//    if (exponent->sign == -1) {
//        temp = base->value.numeric.numerator;
//        base->value.numeric.numerator = base->value.numeric.denominator;
//        base->value.numeric.denominator = temp;
//        exponent->sign = 1;
//    }
//
//    if (exponentiation(&base_result->value.numeric.numerator, base->value.numeric.numerator, exponent->value.numeric.numerator) == RETS_ERROR ||
//        exponentiation(&base_result->value.numeric.denominator, base->value.numeric.denominator, exponent->value.numeric.numerator) == RETS_ERROR) {
//        free_expressions(4, base, exponent, base_result, factor);
//        return RETS_UNCHANGED;
//    }
//
////    int_root(&factor->value.numeric.numerator, &base_result->value.numeric.numerator, base_result->value.numeric.numerator, exponent->value.numeric.denominator);
////    int_root(&factor->value.numeric.denominator, &base_result->value.numeric.denominator, base_result->value.numeric.denominator, exponent->value.numeric.denominator);
//
//
//    factor->sign = sign;
//
//    if (literal_to_double(factor) == 1) {
//        if (literal_to_double(base_result) == 1) {
//            replace_expression(source, new_literal(1, 1, 1));
//            free_expressions(1, base_result);
//        } else {
//            replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                      base_result,
//                                                      exponent));
//        }
//        free_expressions(1, factor);
//    } else {
//        if (literal_to_double(base_result) == 1) {
//            replace_expression(source, factor);
//            free_expressions(1, base_result);
//        } else {
//            replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                      factor,
//                                                      new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                     base_result,
//                                                                     exponent)));
//        }
//    }
//
//    return RETS_CHANGED;
//
//}
//
//return_status simplify_exponentiation(expression* source) {
//
//    if (source->child_count != 2) return RETS_ERROR; //set_errorxx(ERRD_SYNTAX, ERRI_ARGUMENTS, "", false);
//
//    if (source->children[0]->identifier == EXPI_LITERAL && literal_to_double(source->children[0]) == 0 &&
//        source->children[1]->identifier == EXPI_LITERAL && source->children[1]->sign == -1) {
//        return RETS_ERROR; //set_errorxx(ERRD_MATH, ERRI_UNDEFINED_VALUE, "", false);
//    }
//
//    merge_exponentiation_base_exponentiation(source);
//    if (remove_exponentiation_identities(source) == RETS_CHANGED) return RETS_SUCCESS;
//
//    if (expand_exponentiation_base_addition(source) == RETS_CHANGED) return RETS_SUCCESS;
//    if (expand_exponentiation_base_multiplication(source) == RETS_CHANGED) return RETS_SUCCESS;
//
//    if (expand_exponentiation_exponent_addition(source) == RETS_CHANGED) return RETS_SUCCESS;
//    if (simplify_exponentiation_exponent_logarithm(source) == RETS_CHANGED) return RETS_SUCCESS;
//
//    if (numeric_exponentiation(source) == RETS_CHANGED) return RETS_SUCCESS;
//
//    return RETS_SUCCESS;
//
//}
//
//void simplify_abs(expression* source) {
//
//    expression* result;
//
//    if (source->child_count == 1) {
//        if (source->children[0]->identifier == EXPI_LITERAL || symbol_is_constant(source->children[0])) {
//            result = copy_expression(source->children[0]);
//            result->sign = 1;
//            replace_expression(source, result);
//            changed = true;
//        }
//    } else if (source->children[0]->identifier == EXPI_MULTIPLICATION && source->children[0]->child_count == 2 && source->children[0]->children[0]->sign == -1 && symbol_is_constant(source->children[0]->children[1])) {
//        result = copy_expression(source->children[0]);
//        result->children[0]->sign = 1;
//        replace_expression(source, result);
//        changed = true;
//    }
//
//}
//
//uint8_t evaluate_logarithm(expression** result, expression* value, expression* base) {
//
//    if (expressions_are_identical(value, new_literal(1, 0, 1), false)) {
//        return RETS_ERROR; //set_errorxx(ERRD_MATH, ERRI_UNDEFINED_VALUE, "", false);
//    }
//
//    if (expressions_are_identical(value, base, true)) {
//        *result = new_literal(1, 1, 1);
//        return RETS_SUCCESS;
//    }
//
//    if (expressions_are_identical(base, new_symbol("e"), false)) {
//        *result = new_expression(EXPT_FUNCTION, EXPI_LN, 1, copy_expression(value));
//    } else if (expressions_are_identical(base, new_literal(1, 10, 1), false)) {
//        *result = new_expression(EXPT_FUNCTION, EXPI_LOG, 1, copy_expression(value));
//    } else {
//        *result = new_expression(EXPT_FUNCTION, EXPI_LOG, 2,
//                                 copy_expression(value),
//                                 copy_expression(base));
//    }
//
//    return RETS_SUCCESS;
//
//}
//
//uint8_t expand_logarithm(expression* source) {
//
//    uint8_t i;
//    expression* factors;
//    expression* result;
//
//    if (source->identifier != EXPI_LN || source->identifier != EXPI_LN) return RETS_UNCHANGED;
//
//    if (source->children[0]->identifier == EXPI_MULTIPLICATION) {
//
//        result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
//
//        for (i = 0; i < source->children[0]->child_count; i++) {
//            append_child(result, new_expression(EXPT_FUNCTION, source->identifier, 1, copy_expression(source->children[0]->children[i])));
//            if (source->child_count == 2) {
//                append_child(result->children[i], copy_expression(source->children[1]));
//            }
//        }
//
//    } else if (source->children[0]->identifier == EXPI_EXPONENTIATION && expression_is_constant(source->children[0]->children[0]) && source->children[0]->children[0]->sign == 1) {
//
//        result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                copy_expression(source->children[0]->children[1]),
//                                copy_expression(source));
//
//        replace_expression(result->children[1]->children[0], copy_expression(result->children[1]->children[0]->children[0]));
//
//    } else if (source->children[0]->identifier == EXPI_LITERAL && source->children[0]->value.numeric.denominator == 1) {
//
//        result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
//
//        factors = prime_factors(source->children[0]->value.numeric.numerator);
//
//        if (factors->child_count == 1 && factors->children[0]->children[1]->value.numeric.numerator == 1) return RETS_CHANGED;
//
//        for (i = 0; i < factors->child_count; i++) {
//            append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                copy_expression(factors->children[i]->children[1]),
//                                                new_expression(EXPT_FUNCTION, source->identifier, 1, copy_expression(factors->children[i]->children[0]))));
//        }
//
//        free_expression(factors, false);
//
//    } else {
//        return RETS_UNCHANGED;
//    }
//
//    replace_expression(source, result);
//
//    changed = true;
//
//    return RETS_CHANGED;
//
//}
//
///**
//
// @brief Simplifies logarithms
//
// @details
// This function first rewrites all logarithms (ln and log) as
// log(value, base) where base is either e, 10 or an arbitrary
// expression, based on the context. It then calls various
// simplification methods on this logarithm.
//
// @param[in,out] source logarithm to be simplified
//
// @return
// - RETS_SUCCESS on success
// - RETS_ERROR on error
//
// @see
// - TODO
//
// */
//return_status simplify_logarithm(expression* source) {
//
//    expression* value;
//    expression* base;
//
//    if (source->identifier == EXPI_LN && source->child_count == 1) {
//        base = new_symbol("e");
//    } else if (source->identifier == EXPI_LOG && source->child_count == 1) {
//        base = new_literal(1, 10, 1);
//    } else if (source->identifier == EXPI_LOG && source->child_count == 2) {
//        base = copy_expression(source->children[1]);
//    } else {
//        return RETS_ERROR; //set_errorxx(ERRD_SYNTAX, ERRI_ARGUMENTS, "", false);
//    }
//
//    value = copy_expression(source->children[0]);
//
////    TODO: Simplify / add to @see
//
//    replace_expression(source, new_expression(EXPT_FUNCTION, EXPI_LOG, 2,
//                                              value,
//                                              base));
//
//    return RETS_SUCCESS;
//
////    ERROR_CHECK(evaluate_logarithm(&result, source->children[0], base));
////    free_expression(base, false);
////
////    if (expand_logarithm(result) == RETS_CHANGED) {
////        replace_expression(source, result);
////        return RETS_CHANGED;
////    } else {
////        replace_expression(source, result);
////        return RETS_SUCCESS;
////    }
//
//}
//
///**
//
// @brief Evaluates sine expressions
//
// @details
// This function attempts to replace sine expressions with their
// symbolic values using hardcoded values for all multiples of pi/12.
//
// @param[in,out] source expression to evaluate
//
// @return
// - RETS_CHANGED if the source could be simplified
// - RETS_UNCHANGED if the source was left unchanged
//
// */
//return_status evaluate_sin(expression* source) {
//
//    numeric_value value;
//    uintmax_t numerator;
//    expression* result;
//
//    if (n_evaluate(source->children[0], 0) == 0) {
//        replace_expression(source, new_literal(1, 0, 1));
//        return RETS_CHANGED;
//    } else if (n_evaluate(source->children[0], 0) == M_PI) {
//        replace_expression(source, new_literal(1, 0, 1));
//        return RETS_CHANGED;
//    } else if (source->children[0]->identifier == EXPI_MULTIPLICATION &&
//               source->children[0]->child_count == 2 &&
//               source->children[0]->children[0]->identifier == EXPI_LITERAL &&
//               n_evaluate(source->children[0]->children[1], 0) == M_PI) {
//        value = source->children[0]->children[0]->value.numeric;
//    } else {
//        return RETS_UNCHANGED;
//    }
//
//    if (lcm(value.denominator, 12) != 12) return RETS_UNCHANGED;
//
//    if (source->children[0]->children[0]->sign == -1) {
//        numerator = 24 - ((12 * value.numerator) / value.denominator) % 24;
//    } else {
//        numerator = ((12 * value.numerator) / value.denominator) % 24;
//    }
//
//    switch (numerator) {
//        case 0: case 12: case 24:
//            result = new_literal(1, 0, 1);
//            break;
//        case 1: case 11:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(-1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 2: case 10:
//            result = new_literal(1, 1, 2);
//            break;
//        case 3: case 9:
//            result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                    new_literal(1, 2, 1),
//                                    new_literal(-1, 1, 2));
//            break;
//        case 4: case 8:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(1, 1, 2),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        case 5: case 7:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 6:
//            result = new_literal(1, 1, 1);
//            break;
//        case 13: case 23:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(-1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 14: case 22:
//            result = new_literal(-1, 1, 2);
//            break;
//        case 15: case 21:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 2, 1),
//                                                   new_literal(-1, 1, 2)));
//            break;
//        case 16: case 20:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(-1, 1, 2),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        case 17: case 19:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 18:
//            result = new_literal(-1, 1, 1);
//            break;
//        default: return RETS_UNCHANGED;
//    }
//
//    print_expression(result);
//    replace_expression(source, result);
//    return RETS_CHANGED;
//
//}
//
///**
//
// @brief Evaluates cosine expressions
//
// @details
// This function attempts to replace cosine expressions with their
// symbolic values using hardcoded values for all multiples of pi/12.
//
// @param[in,out] source expression to evaluate
//
// @return
// - RETS_CHANGED if the source could be simplified
// - RETS_UNCHANGED if the source was left unchanged
//
// */
//return_status evaluate_cos(expression* source) {
//
//    numeric_value value;
//    uintmax_t numerator;
//    expression* result;
//
//    if (source->children[0]->identifier == EXPI_LITERAL && literal_to_double(source->children[0]) == 0) {
//        replace_expression(source, new_literal(1, 1, 1));
//        return RETS_CHANGED;
//    } else if (source->children[0]->identifier == EXPI_SYMBOL &&
//               source->children[0]->value.symbolic[0] == 'p' &&
//               source->children[0]->value.symbolic[1] == 'i') {
//        replace_expression(source, new_literal(-1, 1, 1));
//        return RETS_CHANGED;
//    } else if (source->children[0]->identifier == EXPI_MULTIPLICATION &&
//               source->children[0]->child_count == 2 &&
//               source->children[0]->children[0]->identifier == EXPI_LITERAL &&
//               source->children[0]->children[1]->identifier == EXPI_SYMBOL &&
//               source->children[0]->children[1]->value.symbolic[0] == 'p' &&
//               source->children[0]->children[1]->value.symbolic[1] == 'i') {
//        value = source->children[0]->children[0]->value.numeric;
//    } else {
//        return RETS_UNCHANGED;
//    }
//
//    if (lcm(value.denominator, 12) != 12) return RETS_UNCHANGED;
//
//    if (source->children[0]->children[0]->sign == -1) {
//        numerator = 24 - ((12 * value.numerator) / value.denominator) % 24;
//    } else {
//        numerator = ((12 * value.numerator) / value.denominator) % 24;
//    }
//
//    switch (numerator) {
//        case 0: case 24:
//            result = new_literal(1, 1, 1);
//            break;
//        case 1: case 23:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 2: case 22:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(1, 1, 2),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        case 3: case 21:
//            result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                    new_literal(1, 2, 1),
//                                    new_literal(-1, 1, 2));
//            break;
//        case 4: case 20:
//            result = new_literal(1, 1, 2);
//            break;
//        case 5: case 19:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(-1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 6: case 18:
//            result = new_literal(1, 0, 1);
//            break;
//        case 7: case 17:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(-1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 8: case 16:
//            result = new_literal(-1, 1, 2);
//            break;
//        case 9: case 15:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 2, 1),
//                                                   new_literal(-1, 1, 2)));
//            break;
//        case 10: case 14:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(-1, 1, 2),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        case 11: case 13:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2)),
//                                                   new_literal(1, 1, 1)),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                  new_literal(1, 2, 1),
//                                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                                 new_literal(1, 2, 1),
//                                                                                 new_literal(1, 1, 2))),
//                                                   new_literal(-1, 1, 1)));
//            break;
//        case 12:
//            result = new_literal(-1, 1, 1);
//            break;
//        default: return RETS_UNCHANGED;
//    }
//
//    replace_expression(source, result);
//    return RETS_CHANGED;
//
//}
//
///**
//
// @brief Evaluates tangent expressions
//
// @details
// This function attempts to replace tangent expressions with their
// symbolic values using hardcoded values for all multiples of pi/12.
//
// @param[in,out] source expression to evaluate
//
// @return
// - RETS_CHANGED if the source could be simplified
// - RETS_UNCHANGED if the source was left unchanged
// - RETS_ERROR if expression value is undefined
//
// */
//return_status evaluate_tan(expression* source) {
//
//    numeric_value value;
//    uintmax_t numerator;
//    expression* result;
//
//    if (source->children[0]->identifier == EXPI_LITERAL && literal_to_double(source->children[0]) == 0) {
//        replace_expression(source, new_literal(1, 0, 1));
//        return RETS_CHANGED;
//    } else if (source->children[0]->identifier == EXPI_SYMBOL &&
//               source->children[0]->value.symbolic[0] == 'p' &&
//               source->children[0]->value.symbolic[1] == 'i') {
//        replace_expression(source, new_literal(1, 0, 1));
//        return RETS_CHANGED;
//    } else if (source->children[0]->identifier == EXPI_MULTIPLICATION &&
//               source->children[0]->child_count == 2 &&
//               source->children[0]->children[0]->identifier == EXPI_LITERAL &&
//               source->children[0]->children[1]->identifier == EXPI_SYMBOL &&
//               source->children[0]->children[1]->value.symbolic[0] == 'p' &&
//               source->children[0]->children[1]->value.symbolic[1] == 'i') {
//        value = source->children[0]->children[0]->value.numeric;
//    } else {
//        return RETS_UNCHANGED;
//    }
//
//    if (lcm(value.denominator, 12) != 12) return RETS_UNCHANGED;
//
//    if (source->children[0]->children[0]->sign == -1) {
//        numerator = 12 - ((12 * value.numerator) / value.denominator) % 12;
//    } else {
//        numerator = ((12 * value.numerator) / value.denominator) % 12;
//    }
//
//    switch (numerator) {
//        case 0: case 12:
//            result = new_literal(1, 0, 1);
//            break;
//        case 1:
//            result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                    new_literal(1, 2, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                   new_literal(-1, 1, 1),
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2))));
//            break;
//        case 2:
//            result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                    new_literal(1, 3, 1),
//                                    new_literal(-1, 1, 2));
//            break;
//        case 3:
//            result = new_literal(1, 1, 1);
//            break;
//        case 4:
//            result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                    new_literal(1, 3, 1),
//                                    new_literal(1, 1, 2));
//            break;
//        case 5:
//            result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                    new_literal(1, 2, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        case 6:
//            return RETS_ERROR; //set_errorxx(ERRD_MATH, ERRI_UNDEFINED_VALUE, "", false);
//        case 7:
//            result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                    new_literal(-1, 2, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                   new_literal(-1, 1, 1),
//                                                   new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                                  new_literal(1, 3, 1),
//                                                                  new_literal(1, 1, 2))));
//            break;
//        case 8:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        case 9:
//            result = new_literal(-1, 1, 1);
//            break;
//        case 10:
//            result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                    new_literal(-1, 1, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(-1, 1, 2)));
//            break;
//        case 11:
//            result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                    new_literal(-1, 2, 1),
//                                    new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
//                                                   new_literal(1, 3, 1),
//                                                   new_literal(1, 1, 2)));
//            break;
//        default: return RETS_UNCHANGED;
//    }
//
//    replace_expression(source, result);
//    return RETS_CHANGED;
//
//}
//
//void evaluate_arcsin(expression* source) {
//
//    double value = n_evaluate(source->children[0], 0);
//    uint8_t factor;
//    expression* result;
//
//    if (isinf(value)) {
//        return;
//    } else if (value == 0) {
//        factor = 0;
//    } else if (value == (sqrt(3) - 1) / (2 * sqrt(2))) {
//        factor = 1;
//    } else if (value == 0.5) {
//        factor = 2;
//    } else if (value == 1 / sqrt(2)) {
//        factor = 3;
//    } else if (value == 0.5 * sqrt(3)) {
//        factor = 4;
//    } else if (value == (sqrt(3) + 1) / (2 * sqrt(2))) {
//        factor = 5;
//    } else if (value == 1) {
//        factor = 6;
//    } else if (value == -(sqrt(3) - 1) / (2 * sqrt(2))) {
//        factor = 13;
//    } else if (value == -0.5) {
//        factor = 14;
//    } else if (value == -1 / sqrt(2)) {
//        factor = 15;
//    } else if (value == -0.5 * sqrt(3)) {
//        factor = 16;
//    } else if (value == -(sqrt(3) + 1) / (2 * sqrt(2))) {
//        factor = 17;
//    } else if (value == -1) {
//        factor = 18;
//    } else {
//        return;
//    }
//
//    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                            new_literal(1, factor, 12),
//                            new_symbol("pi"));
//
//    simplify(result, false);
//    replace_expression(source, result);
//
//}
//
//void simplify_arccos(expression* source) {
//
////    if (expressions_are_identical(source->children[0], new_literal(1, 1, 1), false)) {
////        replace_expression(source, new_literal(1, 1, 1));
////    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
////                                                                                             new_literal(1, 3, 1),
////                                                                                             new_literal(1, 1, 2)),
////                                                                              new_literal(1, 1, 2)), false, false)) {
////        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                  new_symbol("pi"),
////                                                  new_literal(1, 1, 6)));
////    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
////                                                                                             new_literal(1, 2, 1),
////                                                                                             new_literal(1, 1, 2)),
////                                                                              new_literal(1, 1, 2)), false, false)) {
////        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                  new_symbol("pi"),
////                                                  new_literal(1, 1, 4)));
////    } else if (expressions_are_identical(source->children[0], new_literal(1, 1, 2), false)) {
////        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                  new_symbol("pi"),
////                                                  new_literal(1, 1, 3)));
////    } else if (expressions_are_identical(source->children[0], new_literal(1, 0, 1), false)) {
////        replace_expression(source, new_literal(1, 0, 1));
////    } else {
////        return;
////    }
////
////    changed = true;
//
//}
//
//void simplify_arctan(expression* source) {
//
////    if (expressions_are_identical(source->children[0], new_literal(1, 0, 1), false)) {
////        replace_expression(source, new_literal(1, 0, 1));
////    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
////                                                                                             new_literal(1, 3, 1),
////                                                                                             new_literal(1, 1, 2)),
////                                                                              new_literal(1, 1, 3)), false, false)) {
////        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                  new_symbol("pi"),
////                                                  new_literal(1, 1, 6)));
////    } else if (expressions_are_identical(source->children[0], new_literal(1, 1, 1), false)) {
////        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                  new_symbol("pi"),
////                                                  new_literal(1, 1, 4)));
////    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
////                                                                              new_literal(1, 3, 1),
////                                                                              new_literal(1, 1, 2)), false, false)) {
////        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
////                                                  new_symbol("pi"),
////                                                  new_literal(1, 1, 3)));
////    } else {
////        return;
////    }
////
////    changed = true;
//
//}
//
//uint8_t simplify(expression* source, bool recursive) {
//
//    uint8_t i;
//
//    changed = false;
//
//    for (i = 0; i < source->child_count && recursive; i++) {
//        if (source->children[i] == NULL) continue;
//        ERROR_CHECK(simplify(source->children[i], true));
//    }
//
//    any_expression_to_expression(source);
//
//    switch (source->identifier) {
//        case EXPI_LITERAL: simplify_literal(source); break;
//        case EXPI_SYMBOL: break;
//        case EXPI_ADDITION: simplify_addition(source); break;
//        case EXPI_SUBTRACTION: simplify_subtraction(source); break;
//        case EXPI_MULTIPLICATION: simplify_multiplication(source); break;
//        case EXPI_DIVISION: simplify_division(source); break;
//        case EXPI_EXPONENTIATION: ERROR_CHECK(simplify_exponentiation(source)); break;
//        case EXPI_ABS: simplify_abs(source); break;
//        case EXPI_LN:
//        case EXPI_LOG: ERROR_CHECK(simplify_logarithm(source)); break;
//        case EXPI_SIN: evaluate_sin(source); break;
//        case EXPI_COS: evaluate_cos(source); break;
//        case EXPI_TAN: ERROR_CHECK(evaluate_tan(source)); break;
//        case EXPI_ARCSIN: evaluate_arcsin(source); break;
//        case EXPI_ARCCOS: simplify_arccos(source); break;
//        case EXPI_ARCTAN: simplify_arctan(source); break;
//        case EXPI_POLYNOMIAL_SPARSE: break;
//        case EXPI_POLYNOMIAL_DENSE: break;
//        case EXPI_LIST: break;
//        case EXPI_MATRIX: break;
//        case EXPI_EXTENSION: break;
//        default: break;
//    }
//
//    if (changed) ERROR_CHECK(simplify(source, true));
//
//    return RETS_SUCCESS;
//
//}










