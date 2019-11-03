
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

#ifndef expression_h
#define expression_h

#include "symbolic4.h"

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
    EXPI_SYMBOL,
    EXPI_VARIABLE,
    
    EXPI_EQUATION,
    EXPI_ADDITION,
    EXPI_SUBTRACTION,
    EXPI_MULTIPLICATION,
    EXPI_DIVISION,
    EXPI_EXPONENTATION,
    
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
    EXPI_FACTORS,
    EXPI_VALUE,
    
    EXPI_DERIVATIVE,
    EXPI_INTEGRAL,
    EXPI_AREA,
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
    
    EXPI_APPROXIMATE,
    
    EXPI_POLYNOMIAL_SPARSE,
    EXPI_POLYNOMIAL_DENSE,
    EXPI_LIST,
    EXPI_MATRIX,
    EXPI_EXTENSION,
    
    EXPI_COMMA,
    EXPI_LEFT_PARENTHESIS,
    EXPI_RIGHT_PARENTHESIS,
    EXPI_PARSE
    
} expression_identifier;

typedef enum {
    ETSF_NULL,
    ETSF_INFIX,
    ETSF_TIKZ
} expression_to_string_format;

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
    
#ifdef DEBUG_MODE
    struct expression* children[50];
#else
    struct expression** children;
#endif
    
    union {
        char symbolic[10];
        numeric_value numeric;
    } value;
    
} expression;

extern char* keyword_strings[];
extern expression_identifier keyword_identifiers[];

expression* new_expression(expression_type type, expression_identifier identifier, uint8_t child_count, ...);
expression* new_literal(int8_t sign, uintmax_t numerator, uintmax_t denominator);
expression* new_symbol(expression_identifier identifier, const char* value);
expression* new_trigonometic_periodicity(uint8_t period);
expression* copy_expression(const expression* source);
void replace_expression(expression* a, expression* b);
void free_expression(expression* source, bool persistent);
void free_expressions(uint8_t expression_count, ...);
void free_all_except(expression* source);
void append_child(expression* parent, expression* child);
void set_parents(expression* source);
bool expressions_are_identical(const expression* a, expression* b, bool persistent);
bool expressions_are_equivalent(const expression* a, expression* b, bool persistent);
bool expression_is_greater_than(const expression* a, expression* b, bool persistent);
bool expression_is_smaller_than(const expression* a, expression* b, bool persistent);
bool expression_is_constant(const expression* source);
bool symbol_is_constant(const expression* source);
int8_t expression_contains_division(const expression* source);
bool expression_is_reziprocal(const expression* source);
bool expression_is_numerical(const expression* source);
uint8_t count_occurrences(const expression* haystack, expression* needle, bool persistent);
void collect_symbols(expression* symbols, const expression* source);
void remove_child_at_index(expression* source, uint8_t index);
void remove_null_children(expression* source);
void embed_in_list_if_necessary(expression* source);
void merge_nested_lists(expression* source, bool recursive);
uint8_t swap_expressions(expression* a, expression* b);
void replace_occurences(expression* source, const expression* child, const expression* replacement);
void replace_null_with_zero(expression* source);
void order_children(expression* source);
expression* guess_symbol(const expression* source, const char* custom_priorities, uint8_t rank);
expression* get_symbol(const expression* source);
expression* double_to_literal(double source);
double literal_to_double(expression* source);
void literal_to_double_symbol(expression* source);
const char* get_expression_string(expression_identifier identifier);
expression_identifier get_expression_identifier(const char* string);
void expression_to_string(char* buffer, const expression* source, expression_to_string_format format);
#ifdef DEBUG_MODE
void print_expression(const expression* source);
#endif

#endif /* expression_h */
