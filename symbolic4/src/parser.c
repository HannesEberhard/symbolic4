
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

char * strcasestr(const char* haystack, const char* needle);
return_status string_to_literal(expression** result, const char* source);
expression_type get_expression_type(char source);
expression_identifier get_expression_identifier(const char* source);
expression_identifier get_value_identifier(char source);
expression_identifier get_operator_identifier(char source);
expression_identifier get_control_identifier(char source);
void append_multiplication_if_necessary(expression* tokens);
void tokenize_value_buffer(expression* tokens, const char* buffer);
return_status tokenize_value_expression(expression* tokens, uint8_t* index, const char* source);
void tokenize_operator_expression(expression* tokens, char source);
void tokenize_control_expression(expression* tokens, char source);
return_status tokenize(expression** tokens, const char* query);
return_status validate(const expression* tokens);
void merge_expressions_with_operator(expression* output_stack, expression* operator_stack);
void parse_control_expression(expression* tokens, uint8_t* index, expression* output_stack, expression* operator_stack);

char * strcasestr(const char* haystack, const char* needle) {
    
    uint16_t i;
    uint16_t haystack_len = strlen(haystack);
    uint16_t needle_len = strlen(needle);
    
    if (haystack_len < needle_len) return NULL;
    if (haystack <= 0) return NULL;
    if (needle_len <= 0) return (char*) haystack;
    
    for (i = 0; i <= haystack_len - needle_len; i++) {
        if (strncasecmp(haystack + i, needle, needle_len) == 0) {
            return (char*) haystack + i;
        }
    }
    
    return NULL;
    
}

return_status string_to_literal(expression** result, const char* source) {
    
    uint8_t i = 0;
    int8_t sign = 1;
    uintmax_t integer_part = 0;
    uintmax_t fractional_part_numerator = 0;
    uintmax_t fractional_part_denominator = 1;
    uintmax_t divisor;
    
    if (source[0] == '-') {
        sign = -1;
        i = 1;
    }
    
    for ( ; isdigit(source[i]); i++) {
        if (__builtin_mul_overflow(10, integer_part, &integer_part) ||
            __builtin_add_overflow(source[i] - '0', integer_part, &integer_part)) {
            return set_error(ERRI_INTEGER_OVERFLOW_WHILE_PARSING, false, source);
        }
    }
    
    if (source[i] == '.') {
        for (i++; isdigit(source[i]); i++) {
            if (__builtin_mul_overflow(10, fractional_part_numerator, &fractional_part_numerator) ||
                __builtin_add_overflow(source[i] - '0', fractional_part_numerator, &fractional_part_numerator) ||
                __builtin_mul_overflow(10, fractional_part_denominator, &fractional_part_denominator)) {
                return set_error(ERRI_INTEGER_OVERFLOW_WHILE_PARSING, false, source);
            }
        }
    }
    
    if (source[i] != 0) {
        return set_error(ERRI_UNEXPECTED_CHARACTER, false, source[i]);
    }
    
    divisor = gcd(fractional_part_numerator, fractional_part_denominator);
    fractional_part_numerator /= divisor;
    fractional_part_denominator /= divisor;
    
    if (__builtin_mul_overflow(integer_part, fractional_part_denominator, &integer_part) ||
        __builtin_add_overflow(integer_part, fractional_part_numerator, &integer_part)) {
        return set_error(ERRI_INTEGER_OVERFLOW_WHILE_PARSING, false, source);
    }
    
    *result = new_literal(sign, integer_part, fractional_part_denominator);
    
    return RETS_SUCCESS;
    
}

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

expression_identifier get_expression_identifier(const char* source) {
    uint8_t i;
    for (i = 0; keyword_strings[i] && strcasecmp(source, keyword_strings[i]) != 0; i++);
    return keyword_identifiers[i];
}

expression_identifier get_value_identifier(char source) {
    if (isalpha(source)) {
        return EXPI_SYMBOL;
    } else if (isdigit(source) || source == '.') {
        return EXPI_LITERAL;
    } else {
        return EXPI_NULL;
    }
}

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
        return EXPI_EXPONENTIATION;
    } else {
        return EXPI_NULL;
    }
}

expression_identifier get_control_identifier(char source) {
    if (source == ',') {
        return EXPI_COMMA;
    } else if (source == '(') {
        return EXPI_PARENTHESIS_LEFT;
    } else if (source == ')') {
        return EXPI_PARENTHESIS_RIGHT;
    } else {
        return EXPI_NULL;
    }
}

void append_multiplication_if_necessary(expression* tokens) {
    if (tokens->child_count > 0 && (tokens->children[tokens->child_count - 1]->type == EXPT_VALUE || tokens->children[tokens->child_count - 1]->identifier == EXPI_PARENTHESIS_RIGHT)) {
        append_child(tokens, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0));
    }
}

void tokenize_value_buffer(expression* tokens, const char* buffer) {
    
    uint8_t i;
    char* temp_buffer = smart_alloc(strlen(buffer) + 1, sizeof(char));
    char* symbol = smart_alloc(2, sizeof(char));
    char* keyword_occurrence;
    uint8_t keyword_occurrence_position;
    expression_identifier identifier = EXPI_NULL;
    
    strcpy(temp_buffer, buffer);
    memset(symbol, 0, 2);
    
    while ((keyword_occurrence = strcasestr(temp_buffer, "pi")) != 0) {
        strncpy(keyword_occurrence, "~~", 2);
        append_multiplication_if_necessary(tokens);
        append_child(tokens, new_symbol("pi"));
    }
    
    for (i = 0; keyword_strings[i]; i++) {
        if ((keyword_occurrence = strcasestr(temp_buffer, keyword_strings[i])) && strlen(keyword_occurrence) == strlen(keyword_strings[i])) {
            identifier = keyword_identifiers[i];
            break;
        }
    }
    
    keyword_occurrence_position = (identifier != EXPI_NULL) ? keyword_occurrence - temp_buffer : strlen(temp_buffer);
    
    for (i = 0; i < keyword_occurrence_position; i++) {
        if (temp_buffer[i] != '~') {
            append_multiplication_if_necessary(tokens);
            symbol[0] = buffer[i];
            append_child(tokens, new_symbol(symbol));
        }
    }
    
    if (identifier != EXPI_NULL) {
        append_multiplication_if_necessary(tokens);
        append_child(tokens, new_expression(EXPT_FUNCTION, identifier, 0));
    }
    
    smart_free(temp_buffer);
    smart_free(symbol);
    
}

return_status tokenize_value_expression(expression* tokens, uint8_t* index, const char* source) {
    
    char buffer[256];
    expression_identifier identifier;
    expression_identifier current_identifier;
    expression* literal;
    
    memset(buffer, 0, 256);
    current_identifier = get_value_identifier(source[*index]);
    
    while (source[*index]) {
        
        identifier = get_value_identifier(source[*index]);
        
        if (identifier == EXPI_NULL) {
            break;
        }
        
        if (identifier != current_identifier) {
            
            if (current_identifier == EXPI_SYMBOL) {
                tokenize_value_buffer(tokens, buffer);
            } else {
                if (string_to_literal(&literal, buffer) == RETS_ERROR) return RETS_ERROR;
                append_multiplication_if_necessary(tokens);
                append_child(tokens, literal);
            }
            
            memset(buffer, 0, 100);
            current_identifier = identifier;
            
        }
        
        buffer[strlen(buffer)] = source[(*index)++];
        
    }
    
    if (current_identifier == EXPI_SYMBOL) {
        tokenize_value_buffer(tokens, buffer);
    } else {
        if (string_to_literal(&literal, buffer) == RETS_ERROR) return RETS_ERROR;
        append_multiplication_if_necessary(tokens);
        append_child(tokens, literal);
    }
    
    (*index)--;
    
    return RETS_SUCCESS;
    
}

void tokenize_operator_expression(expression* tokens, char source) {
    
    expression_identifier identifier = get_operator_identifier(source);
    
    if (identifier == EXPI_SUBTRACTION &&
        (tokens->child_count == 0 || (tokens->children[tokens->child_count - 1]->type == EXPT_CONTROL && tokens->children[tokens->child_count - 1]->identifier != EXPI_PARENTHESIS_RIGHT))) {
        append_child(tokens, new_literal(-1, 1, 1));
        append_child(tokens, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0));
    } else {
        append_child(tokens, new_expression(EXPT_OPERATION, identifier, 0));
    }
    
}

void tokenize_control_expression(expression* tokens, char source) {
    
    expression_identifier identifier = get_control_identifier(source);
    
    if (identifier == EXPI_PARENTHESIS_LEFT) {
        append_multiplication_if_necessary(tokens);
    }
    
    append_child(tokens, new_expression(EXPT_CONTROL, identifier, 0));
    
}

return_status tokenize(expression** tokens, const char* query) {
    
    uint8_t i;
    *tokens = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; query[i]; i++) {
        
        if (query[i] == ' ') continue;
        
        switch (get_expression_type(query[i])) {
            case EXPT_VALUE:
                if (tokenize_value_expression(*tokens, &i, query) == RETS_ERROR) return RETS_ERROR;
                break;
            case EXPT_OPERATION:
                tokenize_operator_expression(*tokens, query[i]);
                break;
            case EXPT_CONTROL:
                tokenize_control_expression(*tokens, query[i]);
                break;
            default:
                return set_error(ERRI_UNEXPECTED_CHARACTER, false, query[i]);
        }
        
    }
    
    return RETS_SUCCESS;
    
}

return_status validate(const expression* tokens) {
    
    uint8_t i, j;
    int8_t open_parenthesis = 0;
    int8_t temp_open_parenthesis = 0;
    
    if (tokens->child_count == 0) {
        return set_error(ERRI_EMPTY_INPUT, false);
    }
    
    if (tokens->children[0]->type != EXPT_VALUE &&
        tokens->children[0]->type != EXPT_FUNCTION &&
        tokens->children[0]->identifier != EXPI_SUBTRACTION &&
        tokens->children[0]->identifier != EXPI_PARENTHESIS_LEFT) {
        return set_error(ERRI_INVALID_QUERY_START, false, get_expression_string(tokens->children[0]->identifier));
    }
    
    for (i = 0; i < tokens->child_count - 1; i++) {
        if (tokens->children[i]->type == EXPT_VALUE) {
            if (tokens->children[i + 1]->type != EXPT_OPERATION &&
                tokens->children[i + 1]->identifier != EXPI_COMMA &&
                tokens->children[i + 1]->identifier != EXPI_PARENTHESIS_RIGHT) {
                return set_error(ERRI_INVALID_SEQUENCE, false, get_expression_string(tokens->children[i]->identifier), get_expression_string(tokens->children[i + 1]->identifier));
            }
        } else if (tokens->children[i]->type == EXPT_OPERATION) {
            if (tokens->children[i + 1]->type != EXPT_VALUE &&
                tokens->children[i + 1]->type != EXPT_FUNCTION &&
                tokens->children[i + 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                return set_error(ERRI_INVALID_SEQUENCE, false, get_expression_string(tokens->children[i]->identifier), get_expression_string(tokens->children[i + 1]->identifier));
            }
        } else if (tokens->children[i]->type == EXPT_FUNCTION) {
            if (tokens->children[i + 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                return set_error(ERRI_INVALID_SEQUENCE, false, get_expression_string(tokens->children[i]->identifier), get_expression_string(tokens->children[i + 1]->identifier));
            }
        } else if (tokens->children[i]->identifier == EXPI_COMMA) {
            temp_open_parenthesis = 0;
            for (j = i - 1; !(tokens->children[j]->type == EXPT_FUNCTION && temp_open_parenthesis == 1) && j > 0; j--) {
                if (tokens->children[j]->identifier == EXPI_PARENTHESIS_LEFT) temp_open_parenthesis++;
                if (tokens->children[j]->identifier == EXPI_PARENTHESIS_RIGHT) temp_open_parenthesis--;
            }
            if (tokens->children[j]->type != EXPT_FUNCTION || temp_open_parenthesis != 1) {
                return set_error(ERRI_MISPLACED_COMMA, false);
            }
        } else if (tokens->children[i]->identifier == EXPI_PARENTHESIS_LEFT) {
            open_parenthesis++;
            if (tokens->children[i + 1]->type != EXPT_VALUE &&
                tokens->children[i + 1]->type != EXPT_FUNCTION &&
                tokens->children[i + 1]->identifier != EXPI_SUBTRACTION &&
                tokens->children[i + 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                return set_error(ERRI_INVALID_SEQUENCE, false, get_expression_string(tokens->children[i]->identifier), get_expression_string(tokens->children[i + 1]->identifier));
            }
        } else if (tokens->children[i]->identifier == EXPI_PARENTHESIS_RIGHT) {
            open_parenthesis--;
            if (open_parenthesis < 0) {
                return set_error(ERRI_TOO_MANY_RIGHT_PARENTHESIS, false);
            }
            if (tokens->children[i + 1]->type != EXPT_OPERATION &&
                tokens->children[i + 1]->identifier != EXPI_COMMA &&
                tokens->children[i + 1]->identifier != EXPI_PARENTHESIS_RIGHT) {
                return set_error(ERRI_INVALID_SEQUENCE, false, get_expression_string(tokens->children[i]->identifier), get_expression_string(tokens->children[i + 1]->identifier));
            }
        }
    }
    
    if (tokens->children[tokens->child_count - 1]->type != EXPT_VALUE &&
        tokens->children[tokens->child_count - 1]->identifier != EXPI_PARENTHESIS_RIGHT) {
        return set_error(ERRI_INVALID_QUERY_ENDING, false, get_expression_string(tokens->children[tokens->child_count - 1]->identifier));
    }
    
    if (tokens->children[tokens->child_count - 1]->identifier == EXPI_PARENTHESIS_RIGHT) {
        open_parenthesis--;
    }
    
    if (open_parenthesis < 0) {
        return set_error(ERRI_TOO_MANY_RIGHT_PARENTHESIS, false);
    }
    
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
            while (output_stack->children[output_stack->child_count - child_count - 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                child_count++;
            }
            break;
        case EXPT_CONTROL:
            free_expressions(1, operator_stack->children[operator_stack->child_count - 1]);
            operator_stack->child_count--;
            return;
        default:
            break;
    }
    
    for (i = 0; i < child_count; i++) {
        append_child(operator_stack->children[operator_stack->child_count - 1], output_stack->children[output_stack->child_count - child_count + i]);
    }
    
    output_stack->child_count -= child_count;
    
    if (operator_stack->children[operator_stack->child_count - 1]->type == EXPT_FUNCTION) {
        free_expressions(1, output_stack->children[output_stack->child_count - 1]);
        output_stack->child_count--;
    }
    
    append_child(output_stack, operator_stack->children[operator_stack->child_count - 1]);
    operator_stack->child_count--;
    
}

void parse_control_expression(expression* tokens, uint8_t* index, expression* output_stack, expression* operator_stack) {
    
    switch (tokens->children[*index]->identifier) {
            
        case EXPI_COMMA:
            
            while (operator_stack->children[operator_stack->child_count - 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                merge_expressions_with_operator(output_stack, operator_stack);
            }
            
            free_expressions(1, tokens->children[*index]);
            
            break;
            
        case EXPI_PARENTHESIS_LEFT:
            append_child(operator_stack, tokens->children[*index]);
            break;
            
        case EXPI_PARENTHESIS_RIGHT:
            
            while (operator_stack->children[operator_stack->child_count - 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                merge_expressions_with_operator(output_stack, operator_stack);
            }
            
            free_expressions(1, operator_stack->children[operator_stack->child_count - 1]);
            operator_stack->child_count--;
            
            if (operator_stack->child_count > 0) {
                if (operator_stack->children[operator_stack->child_count - 1]->type == EXPT_FUNCTION) {
                    merge_expressions_with_operator(output_stack, operator_stack);
                }
            }
            
            free_expressions(1, tokens->children[*index]);
            
            break;
            
        default:
            break;
            
    }
    
}

return_status parse(expression** result, const char* query) {
    
    uint8_t i;
    expression* tokens;
    expression* output_stack;
    expression* operator_stack;
    
    if (tokenize(&tokens, query) == RETS_ERROR || validate(tokens) == RETS_ERROR) {
        free_expressions(1, tokens);
        return RETS_ERROR;
    }
    
    output_stack = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    operator_stack = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; i < tokens->child_count; i++) {
        
        switch (tokens->children[i]->type) {
                
            case EXPT_VALUE:
                append_child(output_stack, tokens->children[i]);
                break;
                
            case EXPT_OPERATION:
                while (operator_stack->child_count > 0 &&
                       tokens->children[i]->identifier <= operator_stack->children[operator_stack->child_count - 1]->identifier &&
                       tokens->children[i]->identifier != EXPI_EXPONENTIATION &&
                       operator_stack->children[operator_stack->child_count - 1]->identifier != EXPI_PARENTHESIS_LEFT) {
                    merge_expressions_with_operator(output_stack, operator_stack);
                }
                append_child(operator_stack, tokens->children[i]);
                break;
                
            case EXPT_FUNCTION:
                append_child(output_stack, new_expression(EXPT_CONTROL, EXPI_PARENTHESIS_LEFT, 0));
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
    
    *result = copy_expression(output_stack->children[0]);
    free_expressions(2, output_stack, operator_stack);
    
#ifndef DEBUG
    smart_free(tokens->children);
#endif
    
    smart_free(tokens);
    
    return RETS_SUCCESS;
    
}
