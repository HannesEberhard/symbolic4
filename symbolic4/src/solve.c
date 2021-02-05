
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

bool isolation_changed = false;

uint8_t attract_variables(expression* source, expression* variable) {
    
    if (count_occurrences(source->children[1], variable, true) > 0) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                                                 copy_expression(source->children[0]),
                                                                 copy_expression(source->children[1])),
                                                  new_literal(1, 0, 1)));
    }
    
    ERROR_CHECK(simplify(source, true));
    
    return RETS_SUCCESS;
    
}

void subtract_rhs(expression* source) {
    
    expression* result;
    
    if (source->identifier != EXPI_EQUATION) {
        return;
    }
    
    result = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                            copy_expression(source->children[0]),
                            copy_expression(source->children[1]));
    
    simplify(result, true);
    
}

uint8_t isolate_variable_in_addition(expression* source, expression* variable) {
    
    uint8_t i;
    expression* temp = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->children[0]->child_count; i++) {
        if (count_occurrences(source->children[0]->children[i], variable, true) == 0) {
            append_child(temp, source->children[0]->children[i]);
            source->children[0]->children[i] = NULL;
            isolation_changed = true;
        }
    }
    
    replace_expression(source->children[1], new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                                           copy_expression(source->children[1]),
                                                           temp));
    
    ERROR_CHECK(simplify(source, true));
    
    return RETS_SUCCESS;
    
}

uint8_t isolate_variable_in_multiplication(expression* source, expression* variable) {
    
    uint8_t i;
    expression* temp = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
    
    for (i = 0; i < source->children[0]->child_count; i++) {
        if (count_occurrences(source->children[0]->children[i], variable, true) == 0) {
            append_child(temp, source->children[0]->children[i]);
            source->children[0]->children[i] = NULL;
            isolation_changed = true;
        }
    }
    
    replace_expression(source->children[1], new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                                           copy_expression(source->children[1]),
                                                           temp));
    
    ERROR_CHECK(simplify(source, true));
    
    return RETS_SUCCESS;
    
}

uint8_t isolate_variable_in_exponentiation(expression* source, expression* variable) {
    
    expression* result;
    
    if (count_occurrences(source->children[0]->children[0], variable, true) == 0) {
        
        /* variable is in exponent -> logarithms */
        
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                  copy_expression(source->children[0]->children[1]),
                                                  new_expression(EXPT_FUNCTION, EXPI_LOG, 2,
                                                                 copy_expression(source->children[1]),
                                                                 copy_expression(source->children[0]->children[0]))));
        
    } else if (count_occurrences(source->children[0]->children[1], variable, true) == 0) {
        
        /* variable is in base -> roots */
        
        result = new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
                                copy_expression(source->children[1]),
                                new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                               new_literal(1, 1, 1),
                                               copy_expression(source->children[0]->children[1])));
        
        ERROR_CHECK(simplify(result, true));
        
        
        
        if (source->children[0]->children[1]->identifier == EXPI_LITERAL && source->children[0]->children[1]->value.numeric.numerator % 2 == 0 && source->children[0]->children[1]->value.numeric.denominator == 1 &&
            !expressions_are_identical(result, new_literal(1, 0, 1), false)) {
            replace_expression(source, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                                      new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                                     copy_expression(source->children[0]->children[0]),
                                                                     new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                                    new_literal(-1, 1, 1),
                                                                                    copy_expression(result))),
                                                      new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                                     copy_expression(source->children[0]->children[0]),
                                                                     copy_expression(result))));
        } else {
            replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                      copy_expression(source->children[0]->children[0]),
                                                      copy_expression(result)));
        }
        
        free_expression(result, true);
        
    }
    
    ERROR_CHECK(simplify(source, true));
    
    isolation_changed = true;
    
    return RETS_SUCCESS;
    
}

uint8_t isolate_variable_in_ln(expression* source) {
    
    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                              copy_expression(source->children[0]->children[0]),
                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
                                                             new_symbol("e"),
                                                             copy_expression(source->children[1]))));
    
    ERROR_CHECK(simplify(source, true));
    
    isolation_changed = true;
    
    return RETS_SUCCESS;
    
}

uint8_t isolate_variable_in_log(expression* source) {
    
    expression* base;
    
    if (source->children[0]->child_count == 1) {
        base = new_literal(1, 10, 1);
    } else {
        base = copy_expression(source->children[0]->children[1]);
    }
    
    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                              copy_expression(source->children[0]->children[0]),
                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTIATION, 2,
                                                             base,
                                                             copy_expression(source->children[1]))));
    
    ERROR_CHECK(simplify(source, true));
    
    isolation_changed = true;
    
    return RETS_SUCCESS;
    
}

uint8_t isolate_variable_in_trigonometric_function(expression* source) {
    
    uint8_t i;
    expression* right_side;
    
    switch (source->children[0]->identifier) {
            
        case EXPI_SIN:
//            right_side = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
//                                        new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                       new_expression(EXPT_FUNCTION, EXPI_ARCSIN, 1, copy_expression(source->children[1])),
//                                                       new_trigonometric_periodicity(2)),
//                                        new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
//                                                       new_symbol("pi"),
//                                                       new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                                      new_expression(EXPT_FUNCTION, EXPI_ARCSIN, 1, copy_expression(source->children[1])),
//                                                                      new_trigonometric_periodicity(2))));
            break;
            
        case EXPI_COS:
//            right_side = new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
//                                        new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                       new_expression(EXPT_FUNCTION, EXPI_ARCCOS, 1, copy_expression(source->children[1])),
//                                                       new_trigonometric_periodicity(2)),
//                                        new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
//                                                       new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
//                                                                      new_literal(1, 2, 1),
//                                                                      new_symbol("pi")),
//                                                       new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                                                      new_expression(EXPT_FUNCTION, EXPI_ARCCOS, 1, copy_expression(source->children[1])),
//                                                                      new_trigonometric_periodicity(2))));
            break;
            
        case EXPI_TAN:
//            right_side = new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
//                                        new_expression(EXPT_FUNCTION, EXPI_ARCTAN, 1, copy_expression(source->children[1])),
//                                        new_trigonometric_periodicity(1));
            break;
            
        case EXPI_ARCSIN:
            right_side = new_expression(EXPT_FUNCTION, EXPI_SIN, 1, copy_expression(source->children[1]));
            break;
            
        case EXPI_ARCCOS:
            right_side = new_expression(EXPT_FUNCTION, EXPI_COS, 1, copy_expression(source->children[1]));
            break;
            
        case EXPI_ARCTAN:
            right_side = new_expression(EXPT_FUNCTION, EXPI_TAN, 1, copy_expression(source->children[1]));
            break;
            
        default: return RETS_ERROR; //set_errorxx(ERRD_SYSTEM, ERRI_UNEXPECTED_EXPRESSION, "", false);
            
    }
    
    simplify(right_side, true);
    
    if (right_side->identifier == EXPI_LIST) {
        for (i = 0; i < right_side->child_count; i++) {
            replace_expression(right_side->children[i], new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                                       copy_expression(source->children[0]->children[0]),
                                                                       copy_expression(right_side->children[i])));
        }
        replace_expression(source, right_side);
    } else {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EQUATION, 2,
                                                  copy_expression(source->children[0]->children[0]),
                                                  right_side));
    }
    
    isolation_changed = true;
    
    return RETS_SUCCESS;
    
}

uint8_t isolate_variable(expression* source, expression* variable) {
    
    uint8_t i;
    isolation_changed = false;
    
    if (source->identifier == EXPI_LIST) {
        
        for (i = 0; i < source->child_count; i++) {
            ERROR_CHECK(isolate_variable(source->children[i], variable));
        }
        
        return RETS_SUCCESS;
        
    }
    
    switch (source->children[0]->identifier) {
        case EXPI_ADDITION: ERROR_CHECK(isolate_variable_in_addition(source, variable)); break;
        case EXPI_MULTIPLICATION: ERROR_CHECK(isolate_variable_in_multiplication(source, variable)); break;
        case EXPI_EXPONENTIATION: ERROR_CHECK(isolate_variable_in_exponentiation(source, variable)); break;
        case EXPI_LN: ERROR_CHECK(isolate_variable_in_ln(source)); break;
        case EXPI_LOG: ERROR_CHECK(isolate_variable_in_log(source)); break;
        case EXPI_SIN:
        case EXPI_COS:
        case EXPI_TAN:
        case EXPI_ARCSIN:
        case EXPI_ARCCOS:
        case EXPI_ARCTAN: ERROR_CHECK(isolate_variable_in_trigonometric_function(source)); break;
        default: return RETS_UNCHANGED;
    }
    
    if (isolation_changed) ERROR_CHECK(isolate_variable(source, variable));
    
    return RETS_SUCCESS;
    
}

uint8_t handle_right_side_is_zero(expression* source, expression* variable) {
    
    uint8_t i;
    
    if (source->children[0]->identifier == EXPI_MULTIPLICATION) {
        for (i = 0; i < source->children[0]->child_count; i++) {
            if (source->children[0]->children[i]->identifier == EXPI_EXPONENTIATION && expression_is_constant(source->children[0]->children[i]->children[1]) &&
                source->children[0]->children[i]->children[1]->sign == -1) {
                free_expression(source->children[0]->children[i], false);
                source->children[0]->children[i] = NULL;
            }
        }
        simplify(source->children[0], true);
        return RETS_CHANGED;
    }
    
    return RETS_CHANGED;
    
}

uint8_t solve(expression* source, expression* variable) {
    
    if (variable == NULL) {
        variable = guess_symbol(source, "", 0);
        if (variable == NULL) return RETS_SUCCESS;
    } else if (count_occurrences(source, variable, true) == 0) {
        return RETS_ERROR; //set_errorxx(ERRD_SYNTAX, ERRI_ARGUMENTS, "", false);
    }
    
    ERROR_CHECK(attract_variables(source, variable));
    
    if (expressions_are_identical(source->children[1], new_literal(1, 0, 1), false)) {
        switch (handle_right_side_is_zero(source, variable)) {
            case RETS_CHANGED: break;
            case RETS_SUCCESS: return RETS_SUCCESS;
            case RETS_ERROR: return RETS_ERROR;
        }
    }
    
    if (count_occurrences(source, variable, true) == 1) {
        ERROR_CHECK(isolate_variable(source, variable));
    } else {
        ERROR_CHECK(polysolve(source, variable));
    }
    
    return RETS_SUCCESS;
    
}


