
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

char* keyword_strings[] = {
    "=",
    "+",
    "-",
    "*",
    "/",
    "^",
    "abs",
    "ln",
    "log",
    "arcsin",
    "arccos",
    "arctan",
    "sin",
    "cos",
    "tan",
    "simplify",
    "simp",
    "solve",
    "Factors",
    "Fac",
    "Value",
    "Val",
    "Derivative",
    "Deriv",
    "Integral",
    "Int",
    "Area",
    "StationaryPoints",
    "StatPts",
    "InflectionPoints",
    "InflPts",
    "Tangent",
    "Tang",
    "VectorMagnitude",
    "VMag",
    "VectorNormalized",
    "VNormed",
    "Normal",
    "Norm",
    "VectorAngle",
    "VAng",
    "Angle",
    "Ang",
    "VectorDotProduct",
    "VDotP",
    "VectorCrossProduct",
    "VCrossP",
    "VectorTripleProduct",
    "VTripleP",
    "approximate",
    "approx",
    "List",
    "Ls",
    "Vector",
    "Vec",
    ",",
    "(",
    ")",
    "parse",
    NULL
};

expression_identifier keyword_identifiers[] = {
    EXPI_EQUATION,
    EXPI_ADDITION,
    EXPI_SUBTRACTION,
    EXPI_MULTIPLICATION,
    EXPI_DIVISION,
    EXPI_EXPONENTATION,
    EXPI_ABS,
    EXPI_LN,
    EXPI_LOG,
    EXPI_ARCSIN,
    EXPI_ARCCOS,
    EXPI_ARCTAN,
    EXPI_SIN,
    EXPI_COS,
    EXPI_TAN,
    EXPI_SIMPLIFY,
    EXPI_SIMPLIFY,
    EXPI_SOLVE,
    EXPI_FACTORS,
    EXPI_FACTORS,
    EXPI_VALUE,
    EXPI_VALUE,
    EXPI_DERIVATIVE,
    EXPI_DERIVATIVE,
    EXPI_INTEGRAL,
    EXPI_INTEGRAL,
    EXPI_AREA,
    EXPI_STATIONARY_POINTS,
    EXPI_STATIONARY_POINTS,
    EXPI_INFLECTION_POINTS,
    EXPI_INFLECTION_POINTS,
    EXPI_TANGENT,
    EXPI_TANGENT,
    EXPI_V_MAG,
    EXPI_V_MAG,
    EXPI_V_NORMALIZED,
    EXPI_V_NORMALIZED,
    EXPI_NORMAL,
    EXPI_NORMAL,
    EXPI_V_ANGLE,
    EXPI_V_ANGLE,
    EXPI_ANGLE,
    EXPI_ANGLE,
    EXPI_V_DOT_PRODUCT,
    EXPI_V_DOT_PRODUCT,
    EXPI_V_CROSS_PRODUCT,
    EXPI_V_CROSS_PRODUCT,
    EXPI_V_TRIPLE_PRODUCT,
    EXPI_V_TRIPLE_PRODUCT,
    EXPI_APPROXIMATE,
    EXPI_APPROXIMATE,
    EXPI_LIST,
    EXPI_LIST,
    EXPI_LIST,
    EXPI_LIST,
    EXPI_COMMA,
    EXPI_LEFT_PARENTHESIS,
    EXPI_RIGHT_PARENTHESIS,
    EXPI_PARSE,
    EXPI_NULL
};

void expression_to_infix(char* buffer, const expression* souce);
void expression_to_tikz(char* buffer, const expression* source);

/**
 
 @brief Allocates and initializes a new expression with the arguments
 provided
 
 @details
 This function returns a new expression which is initialized with
 the arguments provided and in sign = 1. This function is especially
 useful when being nested to build up more complex expressions in one
 line. Children are not copied.
 
 @param[in] type The expression type.
 @param[in] identifier The expression identifier.
 @param[in] child_count The number of children. It must match the
 actual number of children provided.
 @param[in] ... The child expressions. They may be calls to
 @c new_expression().
 
 @return
 - The initialized expression.
 
 @see
 - new_literal()
 - new_symbol()
 
 */
expression* new_expression(expression_type type, expression_identifier identifier, uint8_t child_count, ...) {
    
    uint8_t i;
    va_list arguments;
    expression* result = smart_alloc(1, sizeof(expression));
    
    result->type = type;
    result->identifier = identifier;
    result->sign = 1;
    
    va_start(arguments, child_count);
    
    for (i = 0; i < child_count; i++) {
        append_child(result, va_arg(arguments, expression*));
    }
    
    va_end(arguments);
    
    return result;
    
}

/**
 
 @brief Allocates and initializes a new literal expression
 
 @details
 This function returns a new literal which is initialized with the
 arguments provided. The result is automatically reduced.
 
 @param[in] sign (either 1 or -1)
 @param[in] numerator The numeric numerator.
 @param[in] denominator The numeric denominator.
 
 @return
 - The initialized literal.
 
 @see
 - new_expression()
 - new_symbol()
 
 */
expression* new_literal(int8_t sign, uintmax_t numerator, uintmax_t denominator) {
    expression* result = new_expression(EXPT_VALUE, EXPI_LITERAL, 0);
    result->sign = sign;
    result->value.numeric.numerator = numerator;
    result->value.numeric.denominator = denominator;
    return result;
}

/**
 
 @brief Allocates and initializes a new symbol/variable expression
 
 @details
 This function returns a new literal which is initialized with the
 argument provided.
 
 @param[in] identifier (either @c EXPI_SYMBOL or @c EXPI_VARIABLE)
 @param[in] value The symbolic value.
 
 @return
 - The initialized symbol.
 
 @see
 - new_expression()
 - new_literal()
 
 */
expression* new_symbol(expression_identifier identifier, const char* value) {
    expression* result = new_expression(EXPT_VALUE, (identifier == EXPI_VARIABLE) ? EXPI_VARIABLE : EXPI_SYMBOL, 0);
    strcpy(result->value.symbolic, value);
    return result;
}

expression* new_trigonometic_periodicity(uint8_t period) {
    
    expression* result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                        new_literal(1, period, 1),
                                        new_symbol(EXPI_SYMBOL, "n"),
                                        new_symbol(EXPI_SYMBOL, "pi"));
    
    return result;
    
}

/**
 
 @brief Returns a deep copy of an expression
 
 @details
 This function recursively (post-order traversal) copies an expression
 and all its children. The copied expression is returned.
 
 @note
 - Null-children are not included in the copy.
 
 @param[in] source The expression to copy.
 
 @return
 - Pointer to the copy.
 
 */
expression* copy_expression(const expression* source) {
    
    uint8_t i;
    expression* result = new_expression(EXPT_NULL, EXPI_NULL, 0);
    
    if (source == NULL) return NULL;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        append_child(result, copy_expression(source->children[i]));
    }
    
    result->parent = source->parent;
    result->type = source->type;
    result->identifier = source->identifier;
    result->child_count = source->child_count;
    result->sign = source->sign;
    result->value = source->value;
    
    return result;
    
}

void replace_expression(expression* a, expression* b) {
    free_expression(a, true);
    *a = *b;
    smart_free(b);
}

void free_expression(expression* source, bool persistent) {
    
    uint8_t i;
    
    if (source == NULL) {
        return;
    }
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        free_expression(source->children[i], false);
        source->children[i] = NULL;
    }
    
    source->child_count = 0;
    
    if (!persistent) {
        smart_free(source);
        source = NULL;
    }
    
}

void free_expressions(uint8_t expression_count, ...) {
    
    uint8_t i;
    va_list arguments;
    
    va_start(arguments, expression_count);
    
    for (i = 0; i < expression_count; i++) {
        free_expression(va_arg(arguments, expression*), false);
    }
    
    va_end(arguments);
    
}

void free_all_except(expression* source) {
    
    uint16_t i;
    expression* temp;
    
    for (i = 0; allocated_pointers[i] != source && i < ALLOCATED_POINTERS_LENGTH; i++);
    allocated_pointers[i] = NULL;
    
    smart_alloc_is_recording = false;
    temp = copy_expression(source);
    smart_free_all();
    
    smart_alloc_is_recording = true;
    *source = *copy_expression(temp);
    free_expression(temp, false);
    
}

/**
 
 @brief Appends a child to an expression
 
 @details
 When @c DEBUG_MODE is set to 0 (production mode),
 the children pointer is resized to hold the new child.
 
 @param[in,out] parent The destination parent.
 @param[in] child The child to be appended.
 
 */
void append_child(expression* parent, expression* child) {
    uint16_t i;
#ifdef DEBUG_MODE
    parent->children[parent->child_count] = child;
    parent->child_count++;
#else
    for (i = 0; allocated_pointers[i] != parent->children && i < ALLOCATED_POINTERS_LENGTH; i++);
    allocated_pointers[i] = NULL;
    parent->children = smart_realloc(parent->children, parent->child_count + 1, sizeof(expression));
    parent->children[parent->child_count] = child;
    parent->child_count++;
#endif
}

void remove_child_at_index(expression* source, uint8_t index) {
    free_expression(source->children[index], false);
    source->children[index] = NULL;
    remove_null_children(source);
}

void remove_null_children(expression* source) {
    
    uint8_t i;
    expression* result = new_expression(source->type, source->identifier, 0);
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        append_child(result, copy_expression(source->children[i]));
    }
    
    replace_expression(source, result);
    
}

void embed_in_list_if_necessary(expression* source) {
    if (source->identifier != EXPI_LIST) {
        replace_expression(source, new_expression(EXPT_STRUCTURE, EXPI_LIST, 1,
                                                  source));
    }
}

void merge_nested_lists(expression* source, bool recursive) {
    
    uint8_t i, j;
    expression* result;
    
    for (i = 0; i < source->child_count && recursive; i++) {
        merge_nested_lists(source->children[i], true);
    }
    
    if (source->identifier != EXPI_LIST) return;
    
    result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->identifier == EXPI_LIST) {
            for (j = 0; j < source->children[i]->child_count; j++) {
                append_child(result, copy_expression(source->children[i]->children[j]));
            }
        } else {
            append_child(result, copy_expression(source->children[i]));
        }
    }
    
    if (result->child_count == 1) {
        replace_expression(result, copy_expression(result->children[0]));
    }
    
    replace_expression(source, result);
    
}

void set_parents(expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        source->children[i]->parent = source;
        set_parents(source->children[i]);
    }
    
}

bool expressions_are_identical(const expression* a, expression* b, bool persistent) {
    
    uint8_t i;
    
    if (a == NULL || b == NULL) {
        return false;
    }
    
    if (a->identifier != b->identifier) {
        if (!persistent) free_expression(b, false);
        return false;
    }
    
    if (a->sign != b->sign) {
        if (!persistent) free_expression(b, false);
        return false;
    }
    
    if (a->identifier == EXPI_LITERAL && (a->value.numeric.numerator != b->value.numeric.numerator || a->value.numeric.denominator != b->value.numeric.denominator)) {
        if (!persistent) free_expression(b, false);
        return false;
    }
    
    if (a->identifier == EXPI_SYMBOL && strcmp(a->value.symbolic, b->value.symbolic) != 0) {
        if (!persistent) free_expression(b, false);
        return false;
    }
    
    if (a->type == EXPT_OPERATION || a->type == EXPT_FUNCTION || a->type == EXPT_STRUCTURE) {
        
        if (a->child_count != b->child_count) {
            if (!persistent) free_expression(b, false);
            return false;
        }
        
        for (i = 0; i < a->child_count; i++) {
            
            if (a->children[i] == NULL && b->children[i] == NULL) continue;
            
            if (a->children[i] == NULL || b->children[i] == NULL) {
                if (!persistent) free_expression(b, false);
                return false;
            }
            
            if (expressions_are_identical(a->children[i], b->children[i], true) == false) {
                if (!persistent) free_expression(b, false);
                return false;
            }
            
        }
        
    }
    
    if (!persistent) free_expression(b, false);
    
    return true;
    
}

bool expressions_are_equivalent(const expression* a, expression* b, bool persistent) {
    
    expression* temp = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                      copy_expression(a),
                                      copy_expression(b));
    
    simplify(temp, true);
    temp->sign = 1;
    
    if (!persistent) free_expression(b, false);
    
    if (expressions_are_identical(temp, new_literal(1, 0, 1), false)) {
        free_expression(temp, false);
        return true;
    } else {
        free_expression(temp, false);
        return false;
    }
    
}

bool expression_is_greater_than(const expression* a, expression* b, bool persistent) {
    
    expression* temp_a = copy_expression(a);
    expression* temp_b = copy_expression(b);
    
    expression* test = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                      temp_a,
                                      temp_b);
    
    simplify(test, true);
    
    if (!persistent) free_expression(b, false);
    
    if (test->identifier == EXPI_LITERAL && test->sign == 1 && literal_to_double(test) != 0) {
        free_expression(test, false);
        return true;
    } else {
        free_expression(test, false);
        return false;
    }
    
}

bool expression_is_smaller_than(const expression* a, expression* b, bool persistent) {
    
    expression* temp_a = copy_expression(a);
    expression* temp_b = copy_expression(b);
    
    expression* test = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                      temp_a,
                                      temp_b);
    
    simplify(test, true);
    
    if (!persistent) free_expression(b, false);
    
    if (test->identifier == EXPI_LITERAL && test->sign == -1) {
        free_expression(test, false);
        return true;
    } else {
        free_expression(test, false);
        return false;
    }
    
}

bool expression_is_constant(const expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (!expression_is_constant(source->children[i])) {
            return false;
        }
    }
    
    if (source->identifier == EXPI_SYMBOL && !symbol_is_constant(source)) {
        return false;
    } else {
        return true;
    }
    
}

bool symbol_is_constant(const expression* source) {
    if (source->identifier == EXPI_SYMBOL &&
        (strcmp(source->value.symbolic, "pi") == 0 ||
         strcmp(source->value.symbolic, "e") == 0 ||
         strcmp(source->value.symbolic, "i") == 0)) {
        return true;
    } else {
        return false;
    }
}

int8_t expression_contains_division(const expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count && source->identifier == EXPI_MULTIPLICATION; i++) {
        if (source->children[i]->identifier == EXPI_EXPONENTATION && source->children[i]->children[1]->sign == -1) {
            return i;
        }
    }
    
    return -1;
    
}

bool expression_is_reziprocal(const expression* source) {
    
    if (source->identifier == EXPI_EXPONENTATION) {
        if (source->children[1]->sign == -1) return true;
    }
    
    return false;
    
}

bool expression_is_numerical(const expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (!expression_is_numerical(source->children[i])) {
            return false;
        }
    }
    
    if (source->identifier == EXPI_SYMBOL) {
        return false;
    } else {
        return true;
    }
    
}

uint8_t count_occurrences(const expression* haystack, expression* needle, bool persistent) {
    
    uint8_t i;
    uint8_t count = 0;
    
    if (expressions_are_identical(haystack, needle, true)) {
        
        if (!persistent) free_expression(needle, false);
        return 1;
        
    } else {
        
        for (i = 0; i < haystack->child_count; i++) {
            if (haystack->children[i] == NULL) continue;
            count += count_occurrences(haystack->children[i], needle, true);
        }
        
        if (!persistent) free_expression(needle, false);
        return count;
        
    }
    
}

void replace_occurences(expression* source, const expression* child, const expression* replacement) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        replace_occurences(source->children[i], child, replacement);
    }
    
    if (expressions_are_identical(source, copy_expression(child), false)) {
        replace_expression(source, copy_expression(replacement));
    }
    
}

void replace_null_with_zero(expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL ||
            (source->children[i]->type == EXPT_STRUCTURE && source->children[i]->child_count == 0)) {
            source->children[i] = new_literal(1, 0, 1);
        } else {
            replace_null_with_zero(source->children[i]);
        }
    }
    
}

uint8_t swap_expressions(expression* a, expression* b) {
    
    expression* temp;
    
    if (a == NULL || b == NULL) return RETS_ERROR;
    
    temp = a;
    b = a;
    b = temp;
    
    return RETS_SUCCESS;
    
}

uint8_t get_symbol_order_score(const expression* source) {
    
    char* occurance;
    
    if (source->identifier != EXPI_SYMBOL || strlen(source->value.symbolic) != 1) return 100;
    
    if ((occurance = strstr(default_priorities, source->value.symbolic))) {
        return (uint8_t) (occurance - default_priorities);
    } else {
        return 100;
    }
    
}

double get_order_score(const expression* source) {
    
    uint8_t i;
    double score;
    
    if (source->identifier == EXPI_LITERAL) {
        score = 1;
    } else if (source->identifier == EXPI_SYMBOL) {
        score = 2 + get_symbol_order_score(source);
    } else if (source->identifier == EXPI_ADDITION || source->identifier == EXPI_MULTIPLICATION) {
        score = 1000;
        for (i = 0; i < source->child_count; i++) score += get_order_score(source->children[i]) / 2;
    } else if (source->identifier == EXPI_EXPONENTATION) {
        score = 20000 + get_order_score(source->children[0]) / 2;
    } else {
        score = 30000;
        for (i = 0; i < source->child_count; i++) score += get_order_score(source->children[i]) / 2;
    }
    
    return score;
    
}

void order_children(expression* source) {
    
    uint8_t i;
    expression** expressions;
    double* scores;
    uint8_t min_index;
    double min_score;
    expression* result;
    
    if (source->identifier != EXPI_ADDITION && source->identifier != EXPI_MULTIPLICATION) return;
    
    expressions = smart_alloc(source->child_count, sizeof(expression));
    scores = smart_alloc(source->child_count, sizeof(double));
    
    result = new_expression(source->type, source->identifier, 0);
    
    remove_null_children(source);
    
    for (i = 0; i < source->child_count; i++) {
        expressions[i] = source->children[i];
        scores[i] = get_order_score(source->children[i]);
    }

    while (result->child_count != source->child_count) {

        min_index = 0;
        min_score = uintmax_max_value();

        for (i = 0; i < source->child_count; i++) {
            if (scores[i] < min_score && scores[i] != 0) {
                min_index = i;
                min_score = scores[i];
            }
        }

        append_child(result, copy_expression(expressions[min_index]));
        scores[min_index] = 0;

    }
    
    smart_free(expressions);
    smart_free(scores);
    replace_expression(source, result);
    
}

void collect_symbols(expression* symbols, const expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        collect_symbols(symbols, source->children[i]);
    }
    
    if (source->identifier == EXPI_SYMBOL || source->identifier == EXPI_VARIABLE) {
        append_child(symbols, copy_expression(source));
    }
    
}

expression* guess_symbol(const expression* source, const char* custom_priorities, uint8_t rank) {
    
    uint8_t i, j;
    expression* symbols = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* symbol;
    
    collect_symbols(symbols, source);
    
    for (i = 0; custom_priorities[i] != '\0'; i++) {
        for (j = 0; j < symbols->child_count; j++) {
            if (strlen(symbols->children[j]->value.symbolic) == 1 && symbols->children[j]->value.symbolic[0] == custom_priorities[i]) {
                if (rank == 0) {
                    symbol = copy_expression(symbols->children[j]);
                    free_expression(symbols, false);
                    return symbol;
                } else {
                    rank--;
                }
            }
        }
    }
    
    for (i = 0; default_priorities[i] != '\0'; i++) {
        for (j = 0; j < symbols->child_count; j++) {
            if (strlen(symbols->children[j]->value.symbolic) == 1 && symbols->children[j]->value.symbolic[0] == default_priorities[i]) {
                if (rank == 0) {
                    symbol = copy_expression(symbols->children[j]);
                    free_expression(symbols, false);
                    return symbol;
                } else {
                    rank--;
                }
            }
        }
    }
    
    if (symbols->child_count != 0) {
        symbol = copy_expression(symbols->children[0]);
        free_expression(symbols, false);
        return symbol;
    }

    free_expression(symbols, false);
    
    return NULL;
    
}

expression* get_symbol(const expression* source) {
    
    expression* symbol;
    
    if (source->identifier == EXPI_POLYNOMIAL_SPARSE) {
        return copy_expression(source->children[0]->children[2]);
    } else if (source->identifier == EXPI_POLYNOMIAL_DENSE) {
        return copy_expression(source->children[0]);
    } else {
        symbol = guess_symbol(source, "", 0);
        if (symbol) {
            return symbol;
        } else {
            return new_symbol(EXPI_SYMBOL, "x");
        }
    }
}

expression* double_to_literal(double source) {
    
    char* buffer = smart_alloc(10, sizeof(char));
    expression* result;
    
    dtoa(buffer, 10, source);
    string_to_literal(&result, buffer);
    
    return result;
    
}

double literal_to_double(expression* source) {
    return source->sign * ((double) source->value.numeric.numerator) / ((double) source->value.numeric.denominator);
}

void literal_to_double_symbol(expression* source) {
    
    uint8_t i;
    char* buffer;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == 0) continue;
        literal_to_double_symbol(source->children[i]);
    }
    
    if (source->identifier == EXPI_LITERAL) {
        buffer = smart_alloc(10, sizeof(char));
        dtoa(buffer, 10, literal_to_double(source));
        replace_expression(source, new_symbol(EXPI_SYMBOL, buffer));
    }
    
}

/**
 
 @brief Returns the keyword string corresponding to the identifier
 
 @details
 The @c keyword_identifiers array is searched for the first occurrence
 of the given identifier. When found, the string from
 @c keyword_strings with the same index is returned. If
 @c use_abbrevations is set to true, the abbrevation (the next string,
 if existent) is returned.
 
 @param[in] identifier The expression identifier.
 
 @return
 - The string corresponding to the identifier.
 
 @see
 - get_expression_identifier()
 
 */
const char* get_expression_string(expression_identifier identifier) {
    
    uint8_t i;
    
    for (i = 0; keyword_identifiers[i] != EXPI_NULL; i++) {
        if (keyword_identifiers[i] == identifier) {
            if (use_abbrevations && keyword_identifiers[i + 1] == identifier) {
                return keyword_strings[i + 1];
            } else {
                return keyword_strings[i];
            }
        }
    }
    
    return "NULL";
    
}

/**
 
 @brief Returns the identifier corresponding to the keyword string
 
 @details
 The @c keyword_strings array is searched for the first occurrence
 of the given string. When found, the identifier from
 @c keyword_identifiers with the same index is returned. This function
 is case insensitive and handles abbrevations.
 
 @param[in] string The keyword string.
 
 @return
 - The identifier corresponding to the string.
 
 @see
 - get_expression_string()
 
 */
expression_identifier get_expression_identifier(const char* string) {
    
    uint8_t i;
    char* lowercase_string = string_to_lower(string);
    char* lowercase_keyword;
    
    for (i = 0; keyword_strings[i] != NULL; i++) {
        lowercase_keyword = string_to_lower(keyword_strings[i]);
        if (strcmp(string, lowercase_keyword) == 0) {
            smart_free(lowercase_string);
            smart_free(lowercase_keyword);
            return keyword_identifiers[i];
        }
        smart_free(lowercase_keyword);
    }
    
    smart_free(lowercase_string);
    
    return EXPI_NULL;
    
}

/**
 
 @brief Serializes an expression into a specified format
 
 @details
 This function calls @c expression_to_infix() or
 @c expression_to_tkiz(), depending on the specified format. It
 automatically converts all polynomials to expressions and sets
 parents. No further simplification is applied. The result can be
 tweaked by changing the bools @c use_abbrevations and @c use_spaces,
 defined in symbolic4.c.
 
 @param[in,out] buffer The buffer where the resulting string shall be
 written into.
 @param[in] source The expression to be serialized.
 @param[in] format The serialization format.
 
 @see
 - expression_to_infix()
 - expression_to_tkiz()
 
 */
void expression_to_string(char* buffer, const expression* source, expression_to_string_format format) {
    
    expression* temp_source = copy_expression(source);
    
    any_expression_to_expression_recursive(temp_source);
    set_parents(temp_source);
    
    switch (format) {
        case ETSF_INFIX: expression_to_infix(buffer, temp_source); break;
        case ETSF_TIKZ: strcat(buffer, "\\begin{tikzpicture}"); expression_to_tikz(buffer, temp_source); strcat(buffer, ";\\end{tikzpicture}"); break;
        default: expression_to_infix(buffer, temp_source); break;
    }
    
    free_expression(temp_source, false);
    
}

void expression_to_infix(char* buffer, const expression* source) {
    
    uint8_t i;
    char temp_buffer[30];
    expression* temp_source = copy_expression(source);
    
    if (temp_source->identifier == EXPI_LITERAL) {
        if (temp_source->value.numeric.denominator == 1) {
            if (temp_source->sign == -1) strcat(buffer, "(-");
            itoa(temp_buffer, temp_source->value.numeric.numerator);
            strcat(buffer, temp_buffer);
            if (temp_source->sign == -1) strcat(buffer, ")");
        } else {
            strcat(buffer, "(");
            if (temp_source->sign == -1) strcat(buffer, "-");
            itoa(temp_buffer, temp_source->value.numeric.numerator);
            strcat(buffer, temp_buffer);
            strcat(buffer, (use_spaces) ? " / " : "/");
            itoa(temp_buffer, temp_source->value.numeric.denominator);
            strcat(buffer, temp_buffer);
            strcat(buffer, ")");
        }
    }
    
    if (temp_source->identifier == EXPI_SYMBOL || temp_source->identifier == EXPI_VARIABLE) {
        strcat(buffer, temp_source->value.symbolic);
    }
    
    if (temp_source->type == EXPT_OPERATION) {
        if (temp_source->parent == NULL || (temp_source->identifier >= temp_source->parent->identifier || temp_source->parent->type != EXPT_OPERATION)) {
            for (i = 0; i < temp_source->child_count; i++) {
                expression_to_infix(buffer, temp_source->children[i]);
                if (use_spaces && i != temp_source->child_count - 1) strcat(buffer, " ");
                if (i != temp_source->child_count - 1) strcat(buffer, get_expression_string(temp_source->identifier));
                if (use_spaces && i != temp_source->child_count - 1) strcat(buffer, " ");
            }
        } else {
            strcat(buffer, "(");
            for (i = 0; i < temp_source->child_count; i++) {
                expression_to_infix(buffer, temp_source->children[i]);
                if (use_spaces && i != temp_source->child_count - 1) strcat(buffer, " ");
                if (i != temp_source->child_count - 1) strcat(buffer, get_expression_string(temp_source->identifier));
                if (use_spaces && i != temp_source->child_count - 1) strcat(buffer, " ");
            }
            strcat(buffer, ")");
        }
    }
    
    if (temp_source->type == EXPT_FUNCTION || temp_source->type == EXPT_STRUCTURE) {
        strcat(buffer, get_expression_string(temp_source->identifier));
        strcat(buffer, "(");
        for (i = 0; i < temp_source->child_count; i++) {
            expression_to_infix(buffer, temp_source->children[i]);
            if (i != temp_source->child_count - 1) strcat(buffer, (use_spaces) ? ", " : ",");
        }
        strcat(buffer, ")");
    }
    
    free_expression(temp_source, false);
    
}

/**
 
 @code
 \tikzset{
     treenode/.style = {align = center, inner sep = 1mm},
     operator_node/.style = {treenode, circle, draw = black},
     function_node/.style = {treenode, rectangle, draw = black},
     node_nil/.style = {treenode, circle, black}
 }
 @endcode
 
 */
void expression_to_tikz(char* buffer, const expression* source) {
    
    uint8_t i;
    char temp_buffer[30];
    expression* temp_source = copy_expression(source);
    
    if (temp_source->identifier == EXPI_LITERAL) {
        strcat(buffer, "child{node{$");
        if (temp_source->sign == -1) strcat(buffer, "- ");
        if (temp_source->value.numeric.denominator == 1) {
            itoa(temp_buffer, temp_source->value.numeric.numerator);
            strcat(buffer, temp_buffer);
        } else {
            strcat(buffer, "\\frac{");
            itoa(temp_buffer, temp_source->value.numeric.numerator);
            strcat(buffer, temp_buffer);
            strcat(buffer, "}{");
            itoa(temp_buffer, temp_source->value.numeric.denominator);
            strcat(buffer, temp_buffer);
            strcat(buffer, "}");
        }
        strcat(buffer, "$}}");
    }
    
    if (temp_source->identifier == EXPI_SYMBOL || temp_source->identifier == EXPI_VARIABLE) {
        strcat(buffer, "child{node{$");
        if (temp_source->sign == -1) strcat(buffer, "- ");
        strcat(buffer, (strcmp(temp_source->value.symbolic, "pi") == 0) ? "\\pi" : temp_source->value.symbolic);
        strcat(buffer, "$}}");
    }
    
    if (temp_source->type == EXPT_OPERATION) {
        strcat(buffer, (temp_source->parent) ? "child{node[operator_node]{$" : "\\node[operator_node]{$");
        if (temp_source->sign == -1) strcat(buffer, "- ");
        strcat(buffer, (temp_source->identifier == EXPI_EXPONENTATION) ? "\\wedge" : get_expression_string(temp_source->identifier));
        strcat(buffer, "$}");
        for (i = 0; i < temp_source->child_count; i++) expression_to_tikz(buffer, temp_source->children[i]);
        if (temp_source->parent) strcat(buffer, "}");
    }
    
    if (temp_source->type == EXPT_FUNCTION || temp_source->type == EXPT_STRUCTURE) {
        strcat(buffer, (temp_source->parent) ? "child{node[function_node]{" : "\\node[function_node]{");
        if (temp_source->sign == -1) strcat(buffer, "- ");
        strcat(buffer, get_expression_string(temp_source->identifier));
        strcat(buffer, "}");
        for (i = 0; i < temp_source->child_count; i++) expression_to_tikz(buffer, temp_source->children[i]);
        if (temp_source->parent) strcat(buffer, "}");
    }
    
    free_expression(temp_source, false);
    
}

#ifdef DEBUG_MODE
void print_expression(const expression* source) {
    char buffer[500];
    memset(buffer, '\0', 500);
    expression_to_string(buffer, source, ETSF_INFIX);
    printf("%s\n", buffer);
}
#endif
