
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

#define KEYWORDS \
X(EXPI_LITERAL, "Literal") \
X(EXPI_FLOATING, "Float") \
X(EXPI_SYMBOL, "Symbol") \
X(EXPI_EQUATION, "=") \
X(EXPI_ADDITION, "+") \
X(EXPI_SUBTRACTION, "-") \
X(EXPI_MULTIPLICATION, "*") \
X(EXPI_DIVISION, "/") \
X(EXPI_EXPONENTIATION, "^") \
X(EXPI_ABS, "abs") \
X(EXPI_LN, "ln") \
X(EXPI_LOG, "log") \
X(EXPI_ARCSIN, "arcsin") \
X(EXPI_ARCCOS, "arccos") \
X(EXPI_ARCTAN, "arctan") \
X(EXPI_SIN, "sin") \
X(EXPI_COS, "cos") \
X(EXPI_TAN, "tan") \
X(EXPI_SIMPLIFY, "simplify") \
X(EXPI_SIMPLIFY, "simp") \
X(EXPI_N_SOLVE, "nsolve") \
X(EXPI_SOLVE, "solve") \
X(EXPI_N_VALUE, "NValue") \
X(EXPI_N_VALUE, "NVal") \
X(EXPI_VALUE, "Value") \
X(EXPI_VALUE, "Val") \
X(EXPI_FACTORS, "Factors") \
X(EXPI_FACTORS, "Fac") \
X(EXPI_N_DERIVATIVE, "NDerivative") \
X(EXPI_N_DERIVATIVE, "NDeriv") \
X(EXPI_DERIVATIVE, "Derivative") \
X(EXPI_DERIVATIVE, "Deriv") \
X(EXPI_N_INTEGRAL, "NIntegral") \
X(EXPI_N_INTEGRAL, "NInt") \
X(EXPI_INTEGRAL, "Integral") \
X(EXPI_INTEGRAL, "Int") \
X(EXPI_N_AREA, "NArea") \
X(EXPI_AREA, "Area") \
X(EXPI_STATIONARY_POINTS, "StationaryPoints") \
X(EXPI_STATIONARY_POINTS, "StatPts") \
X(EXPI_INFLECTION_POINTS, "InflectionPoints") \
X(EXPI_INFLECTION_POINTS, "InflPts") \
X(EXPI_TANGENT, "Tangent") \
X(EXPI_TANGENT, "Tang") \
X(EXPI_V_MAG, "VectorMagnitude") \
X(EXPI_V_MAG, "VectorMag") \
X(EXPI_V_NORMALIZED, "VectorNormalized") \
X(EXPI_V_NORMALIZED, "VNormed") \
X(EXPI_NORMAL, "Normal") \
X(EXPI_V_ANGLE, "VectorAngle") \
X(EXPI_V_ANGLE, "VAng") \
X(EXPI_ANGLE, "Angle") \
X(EXPI_ANGLE, "Ang") \
X(EXPI_V_DOT_PRODUCT, "VectorDotProduct") \
X(EXPI_V_DOT_PRODUCT, "VDotP") \
X(EXPI_V_CROSS_PRODUCT, "VectorCrossProduct") \
X(EXPI_V_CROSS_PRODUCT, "VCrossP") \
X(EXPI_V_TRIPLE_PRODUCT, "VectorTripleProduct") \
X(EXPI_V_TRIPLE_PRODUCT, "VTripleP") \
X(EXPI_LIST, "List") \
X(EXPI_LIST, "Ls") \
X(EXPI_VECTOR, "Vector") \
X(EXPI_VECTOR, "Vec") \
X(EXPI_MATRIX, "Matrix") \
X(EXPI_MATRIX, "Mx") \
X(EXPI_POLYNOMIAL, "Polynomial") \
X(EXPI_POLYNOMIAL, "Poly") \
X(EXPI_COMPLEX, "Complex") \
X(EXPI_COMPLEX, "Cplx") \
X(EXPI_EXTENSION, "Extension") \
X(EXPI_EXTENSION, "Xt") \
X(EXPI_DEGREE, "Degree") \
X(EXPI_DEGREE, "Deg") \
X(EXPI_PARSE, "parse") \
X(EXPI_COMMA, ",") \
X(EXPI_PARENTHESIS_LEFT, "(") \
X(EXPI_PARENTHESIS_RIGHT, ")") \
X(EXPI_NULL, NULL)

expression_identifier keyword_identifiers[] = {
#define X(enum_value, string_value) enum_value,
    KEYWORDS
#undef X
};

char* keyword_strings[] = {
#define X(enum_value, string_value) string_value,
    KEYWORDS
#undef X
};

double get_symbol_order_score(const expression* source);
double get_order_score(const expression* source);
void collect_symbols(expression* symbols, const expression* source);

expression* new_expression(expression_type type, expression_identifier identifier, int child_count, ...) {
    
    uint8_t i;
    va_list args;
    
    expression* result = smart_alloc(1, sizeof(expression));
    result->type = type;
    result->identifier = identifier;
    result->sign = 1;
    result->child_count = child_count;
    
#ifndef DEBUG
    result->children = smart_alloc(child_count, sizeof(expression*));
#endif
    
    va_start(args, child_count);
    
    for (i = 0; i < child_count; i++) {
        result->children[i] = va_arg(args, expression*);
    }
    
    va_end(args);
    
    return result;
    
}

expression* new_literal(int8_t sign, uintmax_t numerator, uintmax_t denominator) {
    expression* result;
    if (denominator == 0) set_error(ERRI_CREATE_LITERAL_WITH_DENOMINATOR_ZERO, true, numerator);
    result = new_expression(EXPT_VALUE, EXPI_LITERAL, 0);
    result->sign = sign == -1 ? -1 : 1;
    result->value.numeric.numerator = numerator;
    result->value.numeric.denominator = denominator == 0 ? 1 : denominator;
    return result;
}

expression* new_symbol(const char* value) {
    expression* result = new_expression(EXPT_VALUE, EXPI_SYMBOL, 0);
    strcpy(result->value.symbolic, value);
    return result;
}

expression* copy_expression(const expression* source) {
    
    uint8_t i;
    expression* result;
    
    if (source == NULL) return NULL;
    
    result = new_expression(source->type, source->identifier, 0);
    result->sign = source->sign;
    result->parent = source->parent;
    result->child_count = source->child_count;
    result->value = source->value;
    
#ifndef DEBUG
    result->children = smart_realloc(result->children, source->child_count, sizeof(expression*));
#endif
    
    for (i = 0; i < source->child_count; i++) {
        result->children[i] = copy_expression(source->children[i]);
    }
    
    return result;
    
}

void free_expressionx(expression* source, bool expression_persistent, bool children_persistent) {
    
    uint8_t i;
    
    if (source == NULL) return;
    
    if (!children_persistent) {
        
        for (i = 0; i < source->child_count; i++) {
            if (source->children[i] != NULL) {
                free_expressionx(source->children[i], false, false);
            }
        }
        
        source->child_count = 0;
        
    }
    
#ifndef DEBUG
    smart_free(source->children);
#endif
    
    if (!expression_persistent) {
        smart_free(source);
    }
    
}

void free_expression(expression* source, bool persistent) {
    free_expressionx(source, persistent, false);
}

void free_expressions(int count, ...) {
    
    uint8_t i;
    va_list arguments;
    
    va_start(arguments, count);
    
    for (i = 0; i < count; i++) {
        free_expressionx(va_arg(arguments, expression*), false, false);
    }
    
    va_end(arguments);
    
}

void replace_expression(expression* a, expression* b) {
    free_expressionx(a, true, false);
    *a = *b;
    smart_free(b);
}

void swap_expressions(expression* a, expression* b) {
    expression temp = *a;
    *a = *b;
    *b = temp;
}

void set_parents(expression* source) {
    
    uint8_t i;
    
    if (source == NULL) return;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] != NULL) {
            source->children[i]->parent = source;
            set_parents(source->children[i]);
        }
    }
    
}

void append_child(expression* parent, expression* child) {
#ifndef DEBUG
    parent->children = smart_realloc(parent->children, parent->child_count + 1, sizeof(expression*));
#endif
    parent->children[parent->child_count] = child;
    parent->child_count++;
}

void remove_child(expression* source, int8_t index) {
    if (index < 0) index += source->child_count;
    if (index < 0 || index >= source->child_count) return;
    free_expressionx(source->children[index], false, false);
    source->children[index] = NULL;
    remove_null_children(source);
}

void remove_null_children(expression* source) {
    
    uint8_t i;
    uint8_t child_count = 0;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] != NULL) {
            source->children[child_count++] = source->children[i];
        }
    }
    
    source->child_count = child_count;
    
#ifndef DEBUG
    source->children = smart_realloc(source->children, child_count, sizeof(expression*));
#endif
    
}

double get_symbol_order_score(const expression* source) {
    
    char* occurrence;
    
    if ((occurrence = strstr(default_priorities, source->value.symbolic))) {
        return occurrence - default_priorities;
    } else {
        return strlen(default_priorities) + strlen(source->value.symbolic);
    }
    
}

double get_order_score(const expression* source) {
    
    uint8_t i;
    double score;
    
    if (source == NULL) {
        return 0;
    } else if (source->identifier == EXPI_LITERAL) {
        score = 1;
    } else if (source->identifier == EXPI_SYMBOL) {
        score = 2 + get_symbol_order_score(source);
    } else {
        score = 100000 * source->identifier;
        for (i = 0; i < source->child_count && (source->type == EXPT_OPERATION || i == 0); i++) {
            score += get_order_score(source->children[i]) / 2;
        }
    }
    
    return score;
    
}

void order_children(expression* source, bool force) {
    
    uint8_t i, j;
    double* scores;
    uint8_t min_index;
    double min_score;
    expression* result;
    
    if ((!force && source->identifier != EXPI_ADDITION && source->identifier != EXPI_MULTIPLICATION) || source->child_count < 2) return;
    
    scores = smart_alloc(source->child_count, sizeof(double));
    result = new_expression(source->type, source->identifier, 0);
    result->sign = source->sign;
    result->parent = source->parent;
    result->child_count = source->child_count;
    result->value = source->value;
    
#ifndef DEBUG
    result->children = smart_realloc(result->children, source->child_count, sizeof(expression*));
#endif
    
    for (i = 0; i < source->child_count; i++) {
        scores[i] = get_order_score(source->children[i]);
    }
    
    for (i = 0; i < source->child_count; i++) {
        
        min_index = 0;
        min_score = (uintmax_t) -1;
        
        for (j = 0; j < source->child_count; j++) {
            if (scores[j] != -1 && scores[j] < min_score) {
                min_index = j;
                min_score = scores[j];
            }
        }
        
        result->children[i] = copy_expression(source->children[min_index]);
        order_children(result->children[i], false);
        scores[min_index] = -1;
        
    }
    
    smart_free(scores);
    replace_expression(source, result);
    
}

void embed_in_list(expression* source) {
    if (source->identifier != EXPI_LIST) {
        replace_expression(source, new_expression(EXPT_STRUCTURE, EXPI_LIST, 1,
                                                  copy_expression(source)));
    }
}

void flatten_structure(expression* source, expression_identifier identifier, int8_t depth) {
    
    uint8_t i, j;
    uint8_t child_count = 0;
    expression* result;
    
    if (depth == 0) return;
    
    for (i = 0; i < source->child_count && depth != 1; i++) {
        flatten_structure(source->children[i], identifier, depth - 1);
    }
    
    if (source->identifier != identifier) return;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->identifier == identifier) {
            child_count += source->children[i]->child_count;
        } else {
            child_count++;
        }
    }
    
    result = new_expression(source->type, source->identifier, 0);
    result->child_count = child_count;
    
#ifndef DEBUG
    result->children = smart_realloc(result->children, child_count, sizeof(expression*));
#endif
    
    child_count = 0;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->identifier == identifier) {
            for (j = 0; j < source->children[i]->child_count; j++) {
                result->children[child_count++] = copy_expression(source->children[i]->children[j]);
            }
        } else {
            result->children[child_count++] = copy_expression(source->children[i]);
        }
    }
    
    replace_expression(source, result);
    
}

bool expressions_are_equal(const expression* a, expression* b, bool persistent) {
    
    bool result;
    
    if (expressions_are_identical(a, b, true)) {
        result = true;
    } else if (compare(a, b, false, true) == CMPR_EQUAL) {
        result = true;
    } else {
        result = false;
    }
    
    if (!persistent) free_expressions(1, b);
    return result;
    
}

bool expressions_are_identical(const expression* a, expression* b, bool persistent) {
    
    uint8_t i;
    bool result = true;
    
    if (a == NULL && b == NULL) {
        return true;
    } else if (a == NULL) {
        if (!persistent) free_expressions(1, b);
        return false;
    }
    
    if (a->type != b->type) {
        result = false;
    } else if (a->identifier != b->identifier) {
        result = false;
    } else if (a->sign != b->sign) {
        result = false;
    } else if (a->child_count != b->child_count) {
        result = false;
    } else if (a->identifier == EXPI_LITERAL && (a->value.numeric.numerator != b->value.numeric.numerator || a->value.numeric.denominator != b->value.numeric.denominator)) {
        result = false;
    } else if (a->identifier == EXPI_SYMBOL && strcmp(a->value.symbolic, b->value.symbolic) != 0) {
        result = false;
    } else {
        for (i = 0; i < a->child_count; i++) {
            if (a->children[i] == NULL && b->children[i] == NULL) continue;
            if (a->children[i] == NULL || b->children[i] == NULL) result = false;
            if (expressions_are_identical(a->children[i], b->children[i], true) == false) result = false;
        }
    }
    
    if (!persistent) free_expressions(1, b);
    
    return result;
    
}

comparison_result compare(const expression* a, expression* b, bool symbolic, bool persistent) {
    
    double a_value;
    double b_value;
    double difference;
    expression* subtraction;
    
    if (!symbolic) {
        a_value = n_evaluate(a, 0);
        b_value = n_evaluate(b, 0);
        if (isfinite(a_value) && isfinite(b_value)) {
            if (!persistent) free_expressions(1, b);
            if (a_value > b_value + DBL_EPSILON) {
                return CMPR_GREATER_THAN;
            } else if (a_value < b_value - DBL_EPSILON) {
                return CMPR_LESS_THAN;
            } else {
                return CMPR_EQUAL;
            }
        }
    }
    
    subtraction = new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                 copy_expression(a),
                                 (persistent) ? copy_expression(b) : b);
    simplify(subtraction, true);
    
    if (!symbolic) {
        difference = n_evaluate(subtraction, 0);
        if (isfinite(difference)) {
            free_expressions(1, subtraction);
            if (difference > DBL_EPSILON) {
                return CMPR_GREATER_THAN;
            } else if (difference < -DBL_EPSILON) {
                return CMPR_LESS_THAN;
            } else {
                return CMPR_EQUAL;
            }
        }
    }
    
    if (subtraction->identifier == EXPI_LITERAL) {
        if (subtraction->value.numeric.numerator == 0) {
            free_expressions(1, subtraction);
            return CMPR_EQUAL;
        } else if (subtraction->sign == 1) {
            free_expressions(1, subtraction);
            return CMPR_GREATER_THAN;
        } else {
            free_expressions(1, subtraction);
            return CMPR_LESS_THAN;
        }
    }
    
    free_expressions(1, subtraction);
    
    return CMPR_NULL;
    
}

uint8_t count_occurrences(const expression* haystack, expression* needle, bool persistent) {
    
    uint8_t i;
    uint8_t count = 0;
    
    if (expressions_are_identical(haystack, needle, true)) {
        count = 1;
    } else if (haystack == NULL) {
        count = 0;
    } else {
        for (i = 0; i < haystack->child_count; i++) {
            count += count_occurrences(haystack->children[i], needle, true);
        }
    }
    
    if (!persistent) free_expressions(1, needle);
    return count;
    
}

void replace_occurrences(expression* source, expression* old, const expression* replacement) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] != NULL) {
            replace_occurrences(source->children[i], old, replacement);
        }
    }
    
    if (expressions_are_identical(source, old, true)) {
        replace_expression(source, copy_expression(replacement));
    }
    
}

void collect_symbols(expression* symbols, const expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] != NULL) {
            collect_symbols(symbols, source->children[i]);
        }
    }
    
    if (source->identifier == EXPI_SYMBOL) {
        append_child(symbols, copy_expression(source));
    }
    
}

expression* guess_symbol(const expression* source, const char* custom_priorities, uint8_t rank) {
    
    uint8_t i, j;
    expression* symbols;
    expression* symbol;
    
    if (source->identifier == EXPI_POLYNOMIAL) {
        return copy_expression(source->children[1]);
    }
    
    symbols = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    collect_symbols(symbols, source);
    
    for (i = 0; custom_priorities[i] != 0; i++) {
        for (j = 0; j < symbols->child_count; j++) {
            if (strlen(symbols->children[j]->value.symbolic) == 1 && symbols->children[j]->value.symbolic[0] == custom_priorities[i]) {
                if (rank == 0) {
                    symbol = copy_expression(symbols->children[j]);
                    free_expressions(1, symbols);
                    return symbol;
                } else {
                    rank--;
                }
            }
        }
    }
    
    for (i = 0; default_priorities[i] != 0; i++) {
        for (j = 0; j < symbols->child_count; j++) {
            if (strlen(symbols->children[j]->value.symbolic) == 1 && symbols->children[j]->value.symbolic[0] == default_priorities[i]) {
                if (rank == 0) {
                    symbol = copy_expression(symbols->children[j]);
                    free_expressions(1, symbols);
                    return symbol;
                } else {
                    rank--;
                }
            }
        }
    }
    
    if (symbols->child_count != 0) {
        symbol = copy_expression(symbols->children[0]);
    } else {
        symbol = new_symbol("x");
    }
    
    free_expressions(1, symbols);
    
    return symbol;
    
}

void print_expression(const expression* source) {
    //#ifdef DEBUG
    char buffer[500];
    memset(buffer, 0, 500);
    serialize(buffer, source, SERF_INFIX);
    printf("%s\n", buffer);
    //#endif
}

// MARK: DEPRECATED

bool expression_is_constant(const expression* source) {
    
    printf("deprecated");
    
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
    
    printf("deprecated");
    
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
    
    printf("deprecated");
    
    uint8_t i;
    
    for (i = 0; i < source->child_count && source->identifier == EXPI_MULTIPLICATION; i++) {
        if (source->children[i]->identifier == EXPI_EXPONENTIATION && source->children[i]->children[1]->sign == -1) {
            return i;
        }
    }
    
    return -1;
    
}

bool expression_is_reziprocal(const expression* source) {
    
    printf("deprecated");
    
    if (source->identifier == EXPI_EXPONENTIATION) {
        if (source->children[1]->sign == -1) return true;
    }
    
    return false;
    
}

bool expression_is_numerical(const expression* source) {
    
    printf("deprecated");
    
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

expression* get_symbol(const expression* source) {
    
    printf("deprecated");
    
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
            return new_symbol("x");
        }
    }
    
}

expression* double_to_literal(double source) {
    
    printf("deprecated");
    
    char* buffer = smart_alloc(10, sizeof(char));
    expression* result;
    
//    ftoa(buffer, 10, source);
//    string_to_literal(&result, buffer);
    
    return result;
    
}

double literal_to_double(expression* source) {
    
    printf("deprecated");
    
    return source->sign * ((double) source->value.numeric.numerator) / ((double) source->value.numeric.denominator);
}

void literal_to_double_symbol(expression* source) {
    
    printf("deprecated");
    
    uint8_t i;
    char* buffer;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == 0) continue;
        literal_to_double_symbol(source->children[i]);
    }
    
    if (source->identifier == EXPI_LITERAL) {
        buffer = smart_alloc(10, sizeof(char));
//        ftoa(buffer, 10, literal_to_double(source));
        replace_expression(source, new_symbol(buffer));
    }
    
}

bool check_literal_value(const expression* source, int8_t sign, uintmax_t numerator, uintmax_t denominator) {
    
    printf("deprecated");
    
    return source != NULL && source->identifier == EXPI_LITERAL && source->sign == sign && source->value.numeric.numerator == numerator && source->value.numeric.denominator == denominator;
}

bool check_symbolic_value(const expression* source, const char* symbol) {
    
    printf("deprecated");
    
    return source != NULL && source->identifier == EXPI_SYMBOL && strcmp(source->value.symbolic, symbol) == 0;
}
