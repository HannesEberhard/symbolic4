
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

/**
 
 @brief Determines the expression type of a character
 
 @details
 It can't distinguish between @c EXPT_VALUE and @c EXPT_FUNCTION.
 
 @param[in] source The character to analyze.
 
 @return
 - The expression type or @c EXPT_NULL when an unexpected character
 was encountered.
 
 */
expression_type get_expression_type(char source) {
    if (isalpha(source) || isdigit(source) || source == '.') {
        return EXPT_VALUE;
    } else if (strchr("=+-/*^", source)) {
        return EXPT_OPERATION;
    } else if (strchr(",()", source)) {
        return EXPT_CONTROL;
    } else {
        return EXPT_NULL;
    }
}

/**
 
 @brief Determines the expression identifier of a character of the
 type EXPT_VALUE
 
 @param[in] source The character to analyze.
 
 @return
 - The expression type or @c EXPT_NULL when an unexpected character
 was encountered.
 
 */
expression_identifier get_value_identifier(char source) {
    if (isalpha(source)) {
        return EXPI_SYMBOL;
    } else if (isdigit(source) || source == '.') {
        return EXPI_LITERAL;
    } else {
        return EXPI_NULL;
    }
}

/**
 
 @brief Determines the expression identifier of a character of the
 type EXPT_OPERATION
 
 @param[in] source The character to analyze.
 
 @return
 - The expression type or @c EXPT_NULL when an unexpected character
 was encountered.
 
 */
expression_identifier get_operator_identifier(char source) {
    if (source == '=') {
        return EXPI_EQUATION;
    } else if (source == '+') {
        return EXPI_ADDITION;
    } else if (source == '-') {
        return EXPI_SUBTRACTION;
    } else if (source == '*') {
        return EXPI_MULTIPLICATION;
    } else if (source == '/') {
        return EXPI_DIVISION;
    } else if (source == '^') {
        return EXPI_EXPONENTATION;
    } else {
        return EXPI_NULL;
    }
}

/**
 
 @brief Determines the expression identifier of a character of the
 type EXPT_CONTROL
 
 @param[in] source The character to analyze.
 
 @return
 - The expression type or @c EXPT_NULL when an unexpected character
 was encountered.
 
 */
expression_identifier get_control_identifier(char source) {
    if (source == ',') {
        return EXPI_COMMA;
    } else if (source == '(') {
        return EXPI_LEFT_PARENTHESIS;
    } else if (source == ')') {
        return EXPI_RIGHT_PARENTHESIS;
    } else {
        return EXPI_NULL;
    }
}

/**
 
 @brief Appends a multiplication expression if necessary
 
 @details
 This function appends a multiplication if
 
 - the token array is not empty
 
 - and the last expression in the token array is either a value or
 a close parenthesis.
 
 @param[in,out] tokens The token array the multiplication should be
 appended to.
 
 */
void append_multiplication_if_necessary(expression* tokens) {
    if (tokens->child_count > 0 && (tokens->children[tokens->child_count - 1]->type == EXPT_VALUE || tokens->children[tokens->child_count - 1]->identifier == EXPI_RIGHT_PARENTHESIS)) {
        append_child(tokens, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0));
    }
}

uint8_t tokenize_buffer(expression* tokens, char* buffer) {
    
    uint8_t i, j;
    char* lowercase_buffer = string_to_lower(buffer);
    char* lowercase_keyword = NULL;
    char* keyword_occurrence = NULL;
    uint8_t keyword_occurrence_position;
    
    /* search for pi */
    while ((keyword_occurrence = strstr(lowercase_buffer, "pi")) != NULL) {
        keyword_occurrence_position = keyword_occurrence - lowercase_buffer;
        append_multiplication_if_necessary(tokens);
        lowercase_buffer[keyword_occurrence_position] = '~';
        lowercase_buffer[keyword_occurrence_position + 1] = '~';
        append_child(tokens, new_symbol(EXPI_SYMBOL, "pi"));
    }
    
    /* search for functions */
    for (i = 0; keyword_strings[i] != NULL; i++) {
        lowercase_keyword = string_to_lower(keyword_strings[i]);
        if ((keyword_occurrence = strstr(lowercase_buffer, lowercase_keyword)) != NULL) {
            if (strlen(keyword_occurrence) != strlen(keyword_strings[i])) continue;
            smart_free(lowercase_keyword);
            break;
        }
        smart_free(lowercase_keyword);
    }
    
    keyword_occurrence_position = (keyword_identifiers[i] != EXPI_NULL) ? keyword_occurrence - lowercase_buffer : strlen(lowercase_buffer);
    
    for (j = 0; j < keyword_occurrence_position; j++) {
        if (lowercase_buffer[j] != '~') {
            append_multiplication_if_necessary(tokens);
            append_child(tokens, new_symbol(EXPI_SYMBOL, &buffer[j]));
            tokens->children[tokens->child_count - 1]->value.symbolic[1] = '\0';
        }
    }
    
    if (keyword_identifiers[i] != EXPI_NULL) {
        append_multiplication_if_necessary(tokens);
        append_child(tokens, new_expression(EXPT_FUNCTION, keyword_identifiers[i], 0));
    }
    
    smart_free(lowercase_buffer);
    
    return RETS_SUCCESS;
    
}

uint8_t string_to_literal(expression** result, const char* source) {
    
    uint8_t i = 0;
    int8_t sign = 1;
    uintmax_t a = 0;
    uintmax_t b = 0;
    uintmax_t c = 1;
    expression* literal;
    
    if (source[0] == '-') {
        sign = -1;
        i++;
    }
    
    for ( ; isdigit(source[i]); i++) {
        a = 10 * a + (source[i] - '0');
//        if (a > pow(2, sizeof(uintmax_t) * 8) / 10) return set_error(ERRD_SYSTEM, ERRI_MAX_INT_VALUE_EXCEEDED, "");
    }
    
    if (source[i] == '.') {
        
        for (i = i + 1; isdigit(source[i]); i++) {
            b = 10 * b + (source[i] - '0');
            c *= 10;
        }
        
        if (source[i] == '.') return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
        
    }
    
    literal = new_literal(1, b, c);
    simplify(literal, false);
    literal->value.numeric.numerator += a * literal->value.numeric.denominator;
    
    *result = literal;
    
    return RETS_SUCCESS;
    
}

uint8_t tokenize_value_expression(expression* tokens, uint8_t* index, const char* source) {
    
    expression_identifier identifier = get_value_identifier(source[*index]);
    expression* literal;
    char buffer[100];
    uint8_t buffer_position = 0;
    
    while (get_value_identifier(source[*index]) == identifier) {
        buffer[buffer_position++] = source[(*index)++];
    }
    
    buffer[buffer_position] = '\0';
    
    if (identifier == EXPI_SYMBOL) {
        ERROR_CHECK(tokenize_buffer(tokens, buffer));
    } else {
        ERROR_CHECK(string_to_literal(&literal, buffer));
        append_multiplication_if_necessary(tokens);
        append_child(tokens, literal);
    }
    
    (*index)--;
    
    return RETS_SUCCESS;
    
}

void tokenize_operator_expression(expression* tokens, char source) {
    
    expression_identifier identifier = get_operator_identifier(source);
    
    if (identifier == EXPI_SUBTRACTION && (tokens->child_count == 0 || tokens->children[tokens->child_count - 1]->type == EXPT_OPERATION || (tokens->children[tokens->child_count - 1]->type == EXPT_CONTROL && tokens->children[tokens->child_count - 1]->identifier != EXPI_RIGHT_PARENTHESIS))) {
        append_child(tokens, new_literal(-1, 1, 1));
        append_child(tokens, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0));
    } else {
        append_child(tokens, new_expression(EXPT_OPERATION, identifier, 0));
    }
    
}

void tokenize_control_expression(expression* tokens, char source) {
    
    expression_identifier identifier = get_control_identifier(source);
    
    if (identifier == EXPI_LEFT_PARENTHESIS) {
        append_multiplication_if_necessary(tokens);
    }
    
    append_child(tokens, new_expression(EXPT_CONTROL, identifier, 0));
    
}

uint8_t tokenize(expression* tokens, const char* query) {
    
    uint8_t i;
    
    for (i = 0; query[i] != '\0'; i++) {
        
        if (query[i] == ' ') continue;
        
        switch (get_expression_type(query[i])) {
            case EXPT_VALUE:
                ERROR_CHECK(tokenize_value_expression(tokens, &i, query));
                break;
            case EXPT_OPERATION:
                tokenize_operator_expression(tokens, query[i]);
                break;
            case EXPT_CONTROL:
                tokenize_control_expression(tokens, query[i]);
                break;
            default:
                return set_error(ERRD_PARSER, ERRI_UNEXPECTED_CHARACTER, "");
        }
        
    }
    
    return RETS_SUCCESS;
    
}

uint8_t validate(expression* tokens) {
    
    uint8_t i;
    int8_t open_parentheses = 0;
    expression* tuple[2];
    
    tuple[1] = tokens->children[0];
    
    for (i = 0; i < tokens->child_count; i++) {
        
        tuple[0] = tuple[1];
        tuple[1] = tokens->children[i];
        
        if (tokens->children[i]->identifier == EXPI_LEFT_PARENTHESIS) open_parentheses++;
        if (tokens->children[i]->identifier == EXPI_RIGHT_PARENTHESIS) open_parentheses--;
        
        if (tuple[0]->type == EXPT_VALUE) {
            
            if (tuple[0] == tuple[1]) continue;
            
            if (tuple[1]->type == EXPT_VALUE) {
                return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
            } else if (tuple[1]->type == EXPT_FUNCTION) {
                return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
            } else if (tuple[1]->identifier == EXPI_LEFT_PARENTHESIS) {
                return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
            }
            
        } else if (tuple[0]->type == EXPT_OPERATION) {
            
            if (tuple[1]->type == EXPT_OPERATION) {
                return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
            } else if (tuple[1]->type == EXPT_CONTROL && tuple[1]->identifier != EXPI_LEFT_PARENTHESIS) {
                return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
            }
            
        } else if (tuple[0]->type == EXPT_FUNCTION) {
            
            if (tuple[0] == tuple[1] && i == 0) continue;
            
            if (tuple[1]->identifier != EXPI_LEFT_PARENTHESIS) {
                return set_error(ERRD_PARSER, ERRI_SYNTAX, "");
            }
            
        }
        
    }
    
    if (open_parentheses < 0) return set_error(ERRD_PARSER, ERRI_PARENTHESIS_MISMATCH, "");
    
    return RETS_SUCCESS;
    
}

void merge_expressions_with_operator(expression* output_stack, expression* operator_stack) {
    
    uint8_t i;
    uint8_t child_count = 0;
    
    switch (operator_stack->children[operator_stack->child_count - 1]->type) {
        case EXPT_OPERATION:
            child_count = 2;
            break;
        case EXPT_FUNCTION:
            while (output_stack->children[output_stack->child_count - child_count - 1]->identifier != EXPI_LEFT_PARENTHESIS) {
                child_count++;
            }
            break;
        case EXPT_CONTROL:
            free_expression(operator_stack->children[operator_stack->child_count - 1], false);
            operator_stack->child_count--;
            return;
        default:
            break;
    }
    
    for (i = 0; i < child_count; i++) {
        append_child(operator_stack->children[operator_stack->child_count - 1], output_stack->children[output_stack->child_count - child_count + i]);
    }
    
    if (operator_stack->children[operator_stack->child_count - 1]->type == EXPT_FUNCTION) {
        output_stack->child_count--;
    }
    
    output_stack->child_count -= child_count;
    append_child(output_stack, operator_stack->children[operator_stack->child_count - 1]);
    operator_stack->child_count--;
    
}

void parse_control_expression(expression* tokens, uint8_t* index, expression* output_stack, expression* operator_stack) {
    
    switch (tokens->children[*index]->identifier) {
            
        case EXPI_COMMA:
            
            while (operator_stack->children[operator_stack->child_count - 1]->identifier != EXPI_LEFT_PARENTHESIS) {
                merge_expressions_with_operator(output_stack, operator_stack);
            }
            
            break;
            
        case EXPI_LEFT_PARENTHESIS:
            append_child(operator_stack, tokens->children[*index]);
            break;
            
        case EXPI_RIGHT_PARENTHESIS:
            
            while (operator_stack->children[operator_stack->child_count - 1]->identifier != EXPI_LEFT_PARENTHESIS) {
                merge_expressions_with_operator(output_stack, operator_stack);
            }
            
            free_expression(operator_stack->children[operator_stack->child_count - 1], false);
            operator_stack->child_count--;
            
            if (operator_stack->child_count > 0) {
                if (operator_stack->children[operator_stack->child_count - 1]->type == EXPT_FUNCTION) {
                    merge_expressions_with_operator(output_stack, operator_stack);
                }
            }
            
            break;
            
        default:
            break;
            
    }
    
}

void parse(expression* tokens) {
    
    uint8_t i;
    expression* output_stack = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* operator_stack = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; i < tokens->child_count; i++) {
        
        switch (tokens->children[i]->type) {
                
            case EXPT_VALUE:
                append_child(output_stack, tokens->children[i]);
                break;
                
            case EXPT_OPERATION:
                while (operator_stack->child_count > 0 &&
                       tokens->children[i]->identifier <= operator_stack->children[operator_stack->child_count - 1]->identifier &&
                       tokens->children[i]->identifier != EXPI_EXPONENTATION &&
                       operator_stack->children[operator_stack->child_count - 1]->identifier != EXPI_LEFT_PARENTHESIS) {
                    merge_expressions_with_operator(output_stack, operator_stack);
                }
                append_child(operator_stack, tokens->children[i]);
                break;
                
            case EXPT_FUNCTION:
                append_child(output_stack, new_expression(EXPT_CONTROL, EXPI_LEFT_PARENTHESIS, 0));
                append_child(operator_stack, tokens->children[i]);
                break;
                
            case EXPT_CONTROL:
                parse_control_expression(tokens, &i, output_stack, operator_stack);
                break;
                
            default:
                break;
                
        }
        
    }
    
    while (operator_stack->child_count > 0) {
        merge_expressions_with_operator(output_stack, operator_stack);
    }
    
    *tokens = *copy_expression(output_stack->children[0]);
    
    free_expression(output_stack, false);
    free_expression(operator_stack, false);
    
}
