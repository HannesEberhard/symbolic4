
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

uint8_t addition_derivative(expression** result, const expression* source, const expression* variable);
uint8_t multiplication_derivative(expression** result, const expression* source, const expression* variable);
uint8_t exponentation_derivative(expression** result, const expression* source, const expression* variable);
uint8_t ln_derivative(expression** result, const expression* source, const expression* variable);
uint8_t log_derivative(expression** result, const expression* source, const expression* variable);
uint8_t trigonometric_derivative(expression** result, const expression* source, const expression* variable);

uint8_t addition_derivative(expression** result, const expression* source, const expression* variable) {
    
    uint8_t i;
    expression* temp_result;
    
    *result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[i]), copy_expression(variable), false));
        append_child(*result, temp_result);
    }
    
    return RETS_SUCCESS;
    
}

uint8_t multiplication_derivative(expression** result, const expression* source, const expression* variable) {
    
    uint8_t i, j;
    expression* temp_result;
    
    *result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        append_child(*result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0));
        for (j = 0; j < source->child_count; j++) {
            if (source->children[i] == NULL) continue;
            if (i == j) {
                ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[j]), copy_expression(variable), false));
                append_child((*result)->children[(*result)->child_count - 1], temp_result);
            } else {
                append_child((*result)->children[(*result)->child_count - 1], copy_expression(source->children[j]));
            }
        }
    }
    
    return RETS_SUCCESS;
    
}

uint8_t exponentation_derivative(expression** result, const expression* source, const expression* variable) {
    
    expression* temp_result;
    
    if (count_occurrences(source->children[1], copy_expression(variable), false) > 0) {
        
        ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[1]), copy_expression(variable), false));
        
        *result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
        append_child(*result, copy_expression(source));
        
        if (count_occurrences(source->children[0], copy_expression(variable), false) > 0) {
            append_child(*result, new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                 new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                new_expression(EXPT_FUNCTION, EXPI_LN, 1, copy_expression(source->children[0])),
                                                                temp_result),
                                                 new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                                copy_expression(source->children[1]),
                                                                copy_expression(source->children[0]))));
        } else if (expressions_are_identical(source->children[0], new_symbol(EXPI_SYMBOL, "e"), false)) {
        } else {
            append_child(*result, new_expression(EXPT_FUNCTION, EXPI_LN, 1, copy_expression(source->children[0])));
        }
        
        ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[1]), copy_expression(variable), false));
        append_child(*result, temp_result);
        
    } else {
        
        ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[0]), copy_expression(variable), false));
        
        *result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                 copy_expression(source->children[1]),
                                 new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                copy_expression(source->children[0]),
                                                new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                               copy_expression(source->children[1]),
                                                               new_literal(-1, 1, 1))),
                                 temp_result);
        
    }
    
    
    return RETS_SUCCESS;
    
}

uint8_t ln_derivative(expression** result, const expression* source, const expression* variable) {
    
    expression* temp_result;
    
    ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[0]), copy_expression(variable), false));
    *result = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                             temp_result,
                             copy_expression(source->children[0]));
    
    return RETS_SUCCESS;
    
}

uint8_t log_derivative(expression** result, const expression* source, const expression* variable) {
    
    expression* temp_result;
    expression* base = (source->child_count == 1) ? new_literal(1, 10, 1) : copy_expression(source->children[1]);
    
    ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[0]), copy_expression(variable), false));
    *result = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                             temp_result,
                             new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                            new_expression(EXPT_FUNCTION, EXPI_LN, 1, base),
                                            copy_expression(source->children[0])));
    
    return RETS_SUCCESS;
    
}

uint8_t trigonometric_derivative(expression** result, const expression* source, const expression* variable) {
    
    expression* temp_result;
    
    *result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
    
    switch (source->identifier) {
        case EXPI_SIN:
            append_child(*result, new_expression(EXPT_FUNCTION, EXPI_COS, 1, copy_expression(source->children[0])));
            break;
        case EXPI_COS:
            append_child(*result, new_literal(-1, 1, 1));
            append_child(*result, new_expression(EXPT_FUNCTION, EXPI_SIN, 1, copy_expression(source->children[0])));
            break;
        case EXPI_TAN:
            append_child(*result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                 new_expression(EXPT_FUNCTION, EXPI_COS, 1, copy_expression(source->children[0])),
                                                 new_literal(-1, 2, 1)));
            break;
        case EXPI_ARCSIN:
            append_child(*result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                 new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                                                new_literal(1, 1, 1),
                                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                               copy_expression(source->children[0]),
                                                                               new_literal(1, 2, 1))),
                                                 new_literal(-1, 1, 2)));
            break;
        case EXPI_ARCCOS:
            append_child(*result, new_literal(-1, 1, 1));
            append_child(*result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                 new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                                                new_literal(1, 1, 1),
                                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                               copy_expression(source->children[0]),
                                                                               new_literal(1, 2, 1))),
                                                 new_literal(-1, 1, 2)));
            break;
        case EXPI_ARCTAN:
            append_child(*result, new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                 new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                               copy_expression(source->children[0]),
                                                                               new_literal(1, 2, 1)),
                                                                new_literal(1, 1, 1))));
            break;
        default: break;
    }
    
    ERROR_CHECK(derivative(&temp_result, copy_expression(source->children[0]), copy_expression(variable), true));
    append_child(*result, temp_result);
    
    return RETS_SUCCESS;
    
}

uint8_t derivative(expression** result, expression* source, expression* variable, bool persistent) {
    
    if (variable == NULL) {
        variable = guess_symbol(source, "", 0);
        if (variable == NULL) {
            *result = new_literal(1, 0, 1);
            return RETS_SUCCESS;
        }
    }
    
    any_expression_to_expression_recursive(source);
    ERROR_CHECK(simplify(source, true));
    
    if (count_occurrences(source, copy_expression(variable), false) == 0) {
        *result = new_literal(1, 0, 1);
        if (!persistent) {
            free_expression(source, false);
            free_expression(variable, false);
        }
        return RETS_SUCCESS;
    }
    
    switch (source->identifier) {
        case EXPI_SYMBOL: *result = (expressions_are_identical(source, copy_expression(variable), false)) ? new_literal(1, 1, 1) : new_literal(1, 0, 1); break;
        case EXPI_ADDITION: addition_derivative(result, source, variable); break;
        case EXPI_MULTIPLICATION: multiplication_derivative(result, source, variable); break;
        case EXPI_EXPONENTATION: exponentation_derivative(result, source, variable); break;
        case EXPI_LN: ln_derivative(result, source, variable); break;
        case EXPI_LOG: log_derivative(result, source, variable); break;
        case EXPI_SIN:
        case EXPI_COS:
        case EXPI_TAN:
        case EXPI_ARCSIN:
        case EXPI_ARCCOS:
        case EXPI_ARCTAN: trigonometric_derivative(result, source, variable); break;
        case EXPI_ABS: return set_error(ERRD_MATH, ERRI_NON_DIFFERENTIABLE, "");
        default: return set_error(ERRD_SYNTAX, ERRI_UNEXPECTED_EXPRESSION, get_expression_string(source->identifier));
    }
    
    ERROR_CHECK(simplify(*result, true));
    
    if (!persistent) {
        free_expression(source, false);
        free_expression(variable, false);
    }
    
    return RETS_SUCCESS;
    
}

uint8_t stationary_points(expression* source, expression* variable) {
    
    uint8_t i;
    expression* first_derivative;
    expression* second_derivative;
    expression* first_derivatives_roots;
    expression* second_derivative_value;
    expression* function_value;
    expression* result;
    
    if (variable == NULL) {
        variable = guess_symbol(source, "", 0);
    }
    
    ERROR_CHECK(derivative(&first_derivative, source, variable, true));
    ERROR_CHECK(derivative(&second_derivative, first_derivative, variable, true));
    
    first_derivatives_roots = new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                             copy_expression(first_derivative),
                                             new_literal(1, 0, 1));
    
    switch (solve(first_derivatives_roots, variable)) {
        case RETS_UNCHANGED: return RETS_UNCHANGED;
        case RETS_ERROR: return RETS_ERROR;
    }
    
    embed_in_list_if_necessary(first_derivatives_roots);
    
    result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; i < first_derivatives_roots->child_count; i++) {
        
        function_value = copy_expression(source);
        replace_occurences(function_value, variable, first_derivatives_roots->children[i]->children[1]);
        
        second_derivative_value = copy_expression(second_derivative);
        replace_occurences(second_derivative_value, variable, first_derivatives_roots->children[i]->children[1]);
        
        if (expressions_are_equivalent(second_derivative_value, new_literal(1, 0, 1), false)) {
            append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                                copy_expression(first_derivatives_roots->children[i]->children[1]),
                                                copy_expression(function_value),
                                                new_literal(1, 0, 1)));
        } else if (expression_is_greater_than(second_derivative_value, new_literal(1, 0, 1), false)) {
            append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                                copy_expression(first_derivatives_roots->children[i]->children[1]),
                                                copy_expression(function_value),
                                                new_literal(-1, 1, 1)));
        } else if (expression_is_smaller_than(second_derivative_value, new_literal(1, 0, 1), false)) {
            append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 3,
                                                copy_expression(first_derivatives_roots->children[i]->children[1]),
                                                copy_expression(function_value),
                                                new_literal(1, 1, 1)));
        } else {
            append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                                copy_expression(first_derivatives_roots->children[i]->children[1]),
                                                copy_expression(function_value)));
        }
        
    }
    
    
    simplify(result, true);
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t function_tangent(expression** result, expression* source, expression* variable, expression* x_value, bool persistent) {
    
    expression* y_value;
    expression* slope;
    
    ERROR_CHECK(derivative(result, source, variable, true));
    
    y_value = copy_expression(source);
    replace_occurences(y_value, variable, x_value);
    
    slope = copy_expression(*result);
    replace_occurences(slope, variable, x_value);
    
    *result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                             new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                            copy_expression(slope),
                                            copy_expression(variable)),
                             new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                            copy_expression(y_value),
                                            new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                           copy_expression(x_value),
                                                           copy_expression(slope))));
    
    free_expression(y_value, false);
    free_expression(slope, false);
    
    if (!persistent) {
        free_expression(source, false);
        free_expression(variable, false);
        free_expression(x_value, false);
    }
    
    ERROR_CHECK(simplify(*result, true));
    
    return RETS_SUCCESS;
    
}

uint8_t function_normal(expression** result, expression* source, expression* variable, expression* x_value, bool persistent) {
    
    expression* y_value;
    expression* slope;
    
    ERROR_CHECK(derivative(result, source, variable, true));
    
    y_value = copy_expression(source);
    replace_occurences(y_value, variable, x_value);
    
    slope = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                           new_literal(-1, 1, 1),
                           copy_expression(*result));
    replace_occurences(slope, variable, x_value);
    
    *result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                             new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                            copy_expression(slope),
                                            copy_expression(variable)),
                             new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                            copy_expression(y_value),
                                            new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                           copy_expression(x_value),
                                                           copy_expression(slope))));
    
    free_expression(y_value, false);
    free_expression(slope, false);
    
    if (!persistent) {
        free_expression(source, false);
        free_expression(variable, false);
        free_expression(x_value, false);
    }
    
    ERROR_CHECK(simplify(*result, true));
    
    return RETS_SUCCESS;
    
}

uint8_t function_intersection_angle(expression** result, expression* g, expression* h){
    
    uint8_t i;
    expression* equation = new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                          copy_expression(g),
                                          copy_expression(h));
    expression* variable = guess_symbol(equation, "", 0);
    expression* g_derivative;
    expression* h_derivative;
    expression* g_slope;
    expression* h_slope;
    
    solve(equation, NULL);
    embed_in_list_if_necessary(equation);
    
    for (i = 0; i < equation->child_count; i++) {
        if (equation->children[i]->children[0]->identifier != EXPI_SYMBOL) {
            *result = new_expression(EXPT_FUNCTION, EXPI_ANGLE, 2,
                                     copy_expression(g),
                                     copy_expression(h));
            return RETS_UNCHANGED;
        }
    }
    
    *result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    derivative(&g_derivative, g, variable, true);
    derivative(&h_derivative, h, variable, true);
    
    for (i = 0; i < equation->child_count; i++) {
        
        g_slope = copy_expression(g_derivative);
        replace_occurences(g_slope, variable, copy_expression(equation->children[i]->children[1]));
        simplify(g_slope, true);
        
        h_slope = copy_expression(h_derivative);
        replace_occurences(h_slope, variable, copy_expression(equation->children[i]->children[1]));
        simplify(h_slope, true);
        
        append_child(*result, new_expression(EXPT_FUNCTION, EXPI_ARCTAN, 1,
                                            new_expression(EXPT_FUNCTION, EXPI_ABS, 1,
                                                           new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                                          new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                                                                         copy_expression(g_slope),
                                                                                         copy_expression(h_slope)),
                                                                          new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                                                         new_literal(1, 1, 1),
                                                                                         new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                                                        copy_expression(g_slope),
                                                                                                        copy_expression(h_slope)))))));
        
        free_expression(g_slope, false);
        free_expression(h_slope, false);
        
        
    }
    
    simplify(*result, true);
    
    merge_nested_lists(*result, false);
    
    return RETS_SUCCESS;
    
}
