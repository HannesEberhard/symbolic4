
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

#ifndef expression_h
#define expression_h

typedef enum {
    CMPR_NULL,
    CMPR_EQUAL,
    CMPR_GREATER_THAN,
    CMPR_LESS_THAN
} comparison_result;

typedef enum {
    EXPT_NULL,
    EXPT_VALUE,
    EXPT_OPERATION,
    EXPT_FUNCTION,
    EXPT_STRUCTURE,
    EXPT_CONTROL
} expression_type;

typedef enum {
    
    EXPI_NULL,
    
    EXPI_LITERAL,
    EXPI_FLOATING,
    EXPI_SYMBOL,
    
    EXPI_EQUATION,
    EXPI_ADDITION,
    EXPI_SUBTRACTION,
    EXPI_MULTIPLICATION,
    EXPI_DIVISION,
    EXPI_EXPONENTIATION,
    
    EXPI_ABS,
    EXPI_LN,
    EXPI_LOG,
    EXPI_SIN,
    EXPI_COS,
    EXPI_TAN,
    EXPI_ARCSIN,
    EXPI_ARCCOS,
    EXPI_ARCTAN,
    
    EXPI_SIMPLIFY,
    EXPI_SOLVE,
    EXPI_N_SOLVE,
    EXPI_VALUE,
    EXPI_N_VALUE,
    EXPI_FACTORS,
    
    EXPI_DERIVATIVE,
    EXPI_N_DERIVATIVE, ///< new
    EXPI_INTEGRAL,
    EXPI_N_INTEGRAL, ///< new
    EXPI_AREA,
    EXPI_N_AREA, ///< new
    EXPI_STATIONARY_POINTS,
    EXPI_INFLECTION_POINTS,
    EXPI_TANGENT,
    EXPI_NORMAL,
    EXPI_ANGLE,
    
    EXPI_V_MAG,
    EXPI_V_NORMALIZED,
    EXPI_V_ANGLE,
    EXPI_V_DOT_PRODUCT,
    EXPI_V_CROSS_PRODUCT,
    EXPI_V_TRIPLE_PRODUCT,
    
    EXPI_LIST,
    EXPI_VECTOR,
    EXPI_MATRIX,
    EXPI_POLYNOMIAL, ///< new
    EXPI_COMPLEX, ///< new
    EXPI_EXTENSION,
    EXPI_DEGREE, ///< new
    
    EXPI_PARSE,
    
    EXPI_COMMA,
    EXPI_PARENTHESIS_LEFT,
    EXPI_PARENTHESIS_RIGHT,
    
    
    EXPI_POLYNOMIAL_SPARSE, ///< deprecated
    EXPI_POLYNOMIAL_DENSE, ///< deprecated
    
} expression_identifier;

typedef struct numeric_value {
    uintmax_t numerator;
    uintmax_t denominator;
} numeric_value;

typedef struct expression {
    
    expression_type type;
    expression_identifier identifier;
    
    int8_t sign;
    struct expression* parent;
    uint8_t child_count;
    
#ifdef DEBUG
    struct expression* children[50];
#else
    struct expression** children;
#endif
    
    union {
        numeric_value numeric;
        double floating;
        char symbolic[8];
    } value;
    
} expression;

extern char* keyword_strings[];
extern expression_identifier keyword_identifiers[];

expression* new_expression(expression_type type, expression_identifier identifier, int child_count, ...);
expression* new_literal(int8_t sign, uintmax_t numerator, uintmax_t denominator);
expression* new_symbol(const char* value);
expression* copy_expression(const expression* source);
void free_expressionx(expression* source, bool expression_persistent, bool children_persistent);
void free_expression(expression* source, bool persistent);
void free_expressions(int count, ...);
void replace_expression(expression* a, expression* b);
void swap_expressions(expression* a, expression* b);
void set_parents(expression* source);
void append_child(expression* parent, expression* child);
void remove_child(expression* source, int8_t index);
void remove_null_children(expression* source);
void order_children(expression* source, bool force);
void embed_in_list(expression* source);
void flatten_structure(expression* source, expression_identifier identifier, int8_t depth);
bool expressions_are_equal(const expression* a, expression* b, bool persistent);
bool expressions_are_identical(const expression* a, expression* b, bool persistent);
comparison_result compare(const expression* a, expression* b, bool symbolic, bool persistent);
uint8_t count_occurrences(const expression* haystack, expression* needle, bool persistent);
void replace_occurrences(expression* source, expression* old, const expression* replacement);
expression* guess_symbol(const expression* source, const char* custom_priorities, uint8_t rank);
void print_expression(const expression* source);

// MARK: DEPRECATED

bool expression_is_constant(const expression* source);
bool symbol_is_constant(const expression* source);
int8_t expression_contains_division(const expression* source);
bool expression_is_reziprocal(const expression* source);
bool expression_is_numerical(const expression* source);
uint8_t count_occurrences(const expression* haystack, expression* needle, bool persistent);
void collect_symbols(expression* symbols, const expression* source);
void remove_null_children(expression* source);
expression* get_symbol(const expression* source);
expression* double_to_literal(double source);
double literal_to_double(expression* source);
void literal_to_double_symbol(expression* source);
bool check_literal_value(const expression* source, int8_t sign, uintmax_t numerator, uintmax_t denominator);
bool check_symbolic_value(const expression* source, const char* symbol);

#endif /* expression_h */
