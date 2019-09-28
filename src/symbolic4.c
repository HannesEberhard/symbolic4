
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

bool use_abbrevations = true;
bool use_spaces = true;
char* default_priorities = "XYZABCDUVWSTPQRJKLMNOFGHEIxyzabcduvwstpqrjklmnofghei";

uint8_t process_equation(expression* source);
uint8_t process_solve(expression* source);
uint8_t process_value(expression* source);
uint8_t process_factors(expression* source);
uint8_t process_derivative(expression* source);
uint8_t process_integral(expression* source);
uint8_t process_stationary_points(expression* source);
uint8_t process_tangent(expression* source);
uint8_t process_normal(expression* source);
uint8_t process_angle(expression* source);
uint8_t process_vector_magnitude(expression* source);
uint8_t process_vector_normalized(expression* source);
uint8_t process_vector_angle(expression* source);
uint8_t process_vector_dot_product(expression* source);
uint8_t process_vector_cross_product(expression* source);
uint8_t process_vector_triple_product(expression* source);
void process_approximate(expression* source);

uint8_t symbolic4(char* buffer, const char* query) {
    
    expression* root;
    
    allocated_pointers = calloc(ALLOCATED_POINTERS_LENGTH, sizeof(void*));
    memset(allocated_pointers, 0, ALLOCATED_POINTERS_LENGTH);
    buffer[0] = '\0';
    
    root = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);

    ERROR_CHECK(tokenize(root, query));
    ERROR_CHECK(validate(root));
    parse(root);

    if (root->identifier == EXPI_PARSE) {
        expression_to_string(buffer, root->children[0], (root->child_count == 2) ? (uint8_t) root->children[1]->value.numeric.numerator : ETSF_INFIX);
    } else {
        ERROR_CHECK(process(root, true));
        expression_to_string(buffer, root, ETSF_INFIX);
    }

    free_expression(root, false);
    smart_free_all();
    free(allocated_pointers);
    
    return RETS_SUCCESS;
    
}

uint8_t process(expression* source, bool recursive) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count && recursive; i++) {
        if (source->children[i] == NULL) continue;
        ERROR_CHECK(process(source->children[i], true));
    }
    
    if ((source->type == EXPT_FUNCTION || source->identifier == EXPT_STRUCTURE) && source->child_count == 0) {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(source->identifier));
    }
    
    switch (source->identifier) {
        case EXPI_EQUATION: ERROR_CHECK(process_equation(source)); break;
        case EXPI_SOLVE: ERROR_CHECK(process_solve(source)); break;
        case EXPI_FACTORS: ERROR_CHECK(process_factors(source)); break;
        case EXPI_VALUE: process_value(source); break;
        case EXPI_DERIVATIVE: ERROR_CHECK(process_derivative(source)); break;
        case EXPI_INTEGRAL: ERROR_CHECK(process_integral(source)); break;
        case EXPI_TANGENT: ERROR_CHECK(process_tangent(source)); break;
        case EXPI_NORMAL: ERROR_CHECK(process_normal(source)); break;
        case EXPI_ANGLE: ERROR_CHECK(process_angle(source)); break;
        case EXPI_V_MAG: ERROR_CHECK(process_vector_magnitude(source)); break;
        case EXPI_V_NORMALIZED: ERROR_CHECK(process_vector_normalized(source)); break;
        case EXPI_V_ANGLE: ERROR_CHECK(process_vector_angle(source)); break;
        case EXPI_V_DOT_PRODUCT: ERROR_CHECK(process_vector_dot_product(source)); break;
        case EXPI_V_CROSS_PRODUCT: ERROR_CHECK(process_vector_cross_product(source)); break;
        case EXPI_V_TRIPLE_PRODUCT: ERROR_CHECK(process_vector_triple_product(source)); break;
        case EXPI_APPROXIMATE: process_approximate(source); break;
        default: ERROR_CHECK(simplify(source, !recursive)); break;
    }
    
    return RETS_SUCCESS;
    
}

uint8_t process_equation(expression* source) {
    if (source->child_count != 2) return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    ERROR_CHECK(solve(source, NULL));
    return RETS_SUCCESS;
}

uint8_t process_solve(expression* source) {
    
    if (source->children[0]->identifier != EXPI_EQUATION) return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    
    if (source->child_count == 1) {
        ERROR_CHECK(solve(source->children[0], NULL));
    } else if (source->child_count == 2 && source->children[1]->identifier == EXPI_SYMBOL) {
        ERROR_CHECK(solve(source->children[0], source->children[1]));
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    }
    
    replace_expression(source, copy_expression(source->children[0]));
    
    return RETS_SUCCESS;
    
}

uint8_t process_factors(expression* source) {
    
    expression* result;
    
    if (source->child_count != 1) return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    
    if (source->children[0]->identifier == EXPI_LITERAL) {
        if (source->children[0]->sign == 1 && source->children[0]->value.numeric.denominator == 1) {
            result = prime_factors(source->children[0]->value.numeric.numerator);
        } else {
            return RETS_UNCHANGED;
        }
    } else {
        result = 0;
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_value(expression* source) {
    
    uint8_t i;
    expression* symbol;
    
    if (source->child_count < 2) {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    } else if (source->children[1]->identifier != EXPI_EQUATION) {
        symbol = guess_symbol(source->children[0], "", 0);
        replace_occurences(source->children[0], symbol, source->children[1]);
        free_expression(symbol, false);
    } else {
        for (i = 1; i < source->child_count; i++) {
            if (source->children[i]->identifier == EXPI_EQUATION) {
                replace_occurences(source->children[0], source->children[i]->children[0], source->children[i]->children[1]);
            } else {
                return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
            }
        }
    }
    
    replace_expression(source, copy_expression(source->children[0]));
    simplify(source, true);
    
    return RETS_SUCCESS;
    
}

uint8_t process_derivative(expression* source) {
    
    expression* result;
    
    if (source->child_count == 1) {
        ERROR_CHECK(derivative(&result, source->children[0], NULL, true));
    } else if (source->child_count == 2 && source->children[1]->identifier == EXPI_SYMBOL) {
        ERROR_CHECK(derivative(&result, source->children[0], source->children[1], true));
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_DERIVATIVE));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_integral(expression* source) {
    
    expression* result;
    
    if (source->child_count == 1) {
        ERROR_CHECK(antiderivative(&result, source->children[0], NULL, true));
    } else if (source->child_count == 2 && source->children[1]->identifier == EXPI_SYMBOL) {
        ERROR_CHECK(antiderivative(&result, source->children[0], source->children[1], true));
    } else if (source->child_count == 3) {
        ERROR_CHECK(definite_integral(&result, source->children[0], NULL, source->children[1], source->children[2]));
    }  else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_DERIVATIVE));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_stationary_points(expression* source) {
    
    expression* result = copy_expression(source->children[0]);
    
    if (source->child_count == 1) {
        stationary_points(result, NULL);
    } else if (source->child_count == 2) {
        stationary_points(result, source->children[1]);
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_DERIVATIVE));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_tangent(expression* source) {
    
    expression* variable;
    expression* result;
    
    if (source->child_count == 2) {
        variable = guess_symbol(source, "", 0);
        ERROR_CHECK(function_tangent(&result, source->children[0], variable, source->children[1], true));
    } else if (source->child_count == 3) {
        ERROR_CHECK(function_tangent(&result, source->children[0], source->children[1], source->children[2], true));
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_TANGENT));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_normal(expression* source) {
    
    expression* variable;
    expression* result;
    
    if (source->child_count == 2) {
        variable = guess_symbol(source, "", 0);
        ERROR_CHECK(function_normal(&result, source->children[0], variable, source->children[1], true));
    } else if (source->child_count == 3) {
        ERROR_CHECK(function_normal(&result, source->children[0], source->children[1], source->children[2], true));
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_TANGENT));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_angle(expression* source) {
    
    expression* result;
    
    if (source->child_count != 2) return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_TANGENT));
    ERROR_CHECK(function_intersection_angle(&result, source->children[0], source->children[1]));
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_vector_magnitude(expression* source) {
    
    expression* result;
    
    if (source->child_count == 1 && source->children[0]->identifier == EXPI_LIST) {
        ERROR_CHECK(vector_magnitude(&result, source->children[0], true));
    } else {
        ERROR_CHECK(vector_magnitude(&result, source, true));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_vector_normalized(expression* source) {
    
    expression* result;
    expression* magnitude;
    
    if (source->children[0]->identifier == EXPI_LIST) {
        if (source->child_count == 1) {
            magnitude = new_literal(1, 1, 1);
        } else if (source->child_count == 2) {
            if (source->children[1]->identifier == EXPI_LIST) {
                vector_magnitude(&magnitude, source->children[1], true);
            } else {
                magnitude = copy_expression(source->children[1]);
            }
        } else {
            return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_V_NORMALIZED));
        }
        ERROR_CHECK(vector_normalized(&result, source->children[0], magnitude, true));
    } else {
        magnitude = new_literal(1, 1, 1);
        ERROR_CHECK(vector_normalized(&result, source, magnitude, true));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_vector_angle(expression* source) {
    
    expression* result;
    
    if (source->child_count == 2 && source->children[0]->identifier == EXPI_LIST && source->children[1]->identifier == EXPI_LIST) {
        if (source->children[0]->child_count == source->children[1]->child_count) {
            ERROR_CHECK(vector_angle(&result, source->children[0], source->children[1], true));
        } else {
            return set_error(ERRD_VECTOR, ERRI_VECTOR_DIMENSIONS, "");
        }
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_V_NORMALIZED));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_vector_dot_product(expression* source) {
    
    expression* result;
    
    if (source->child_count == 2 && source->children[0]->identifier == EXPI_LIST && source->children[1]->identifier == EXPI_LIST) {
        if (source->children[0]->child_count == source->children[1]->child_count) {
            ERROR_CHECK(vector_dot_product(&result, source->children[0], source->children[1], true));
        } else {
            return set_error(ERRD_VECTOR, ERRI_VECTOR_DIMENSIONS, "");
        }
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_V_NORMALIZED));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_vector_cross_product(expression* source) {
    
    expression* result;
    
    if (source->child_count == 2 && source->children[0]->identifier == EXPI_LIST && source->children[1]->identifier == EXPI_LIST) {
        if (source->children[0]->child_count == 3 && source->children[1]->child_count == 3) {
            ERROR_CHECK(vector_cross_product(&result, source->children[0], source->children[1], true));
        } else {
            return set_error(ERRD_VECTOR, ERRI_VECTOR_DIMENSIONS, "");
        }
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_V_NORMALIZED));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

uint8_t process_vector_triple_product(expression* source) {
    
    expression* result;
    
    if (source->child_count == 3 && source->children[0]->identifier == EXPI_LIST && source->children[1]->identifier == EXPI_LIST && source->children[2]->identifier == EXPI_LIST) {
        if (source->children[0]->child_count == 3 && source->children[1]->child_count == 3 && source->children[2]->child_count == 3) {
            ERROR_CHECK(vector_triple_product(&result, source->children[0], source->children[1], source->children[2], true));
        } else {
            return set_error(ERRD_VECTOR, ERRI_VECTOR_DIMENSIONS, "");
        }
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, get_expression_string(EXPI_V_NORMALIZED));
    }
    
    replace_expression(source, result);
    
    return RETS_SUCCESS;
    
}

void process_approximate(expression* source) {
    
    approximate(source);
    literal_to_double_symbol(source);
    
    replace_expression(source, copy_expression(source->children[0]));
    
    return;
    
}
