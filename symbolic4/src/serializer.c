
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

uint8_t ftoa_(char* buffer, double value, bool add_parentheses);
bool serialize_value_as_fraction(numeric_value value);
void serialize_infix_literal(char* buffer, const expression* source);
void serialize_infix_operation(char* buffer, const expression* source);
void serialize_infix_function_structure(char* buffer, const expression* source);
void serialize_infix(char* buffer, const expression* source);
void serialize_tex_literal(char* buffer, const expression* source);
void serialize_tex_symbol(char* buffer, const expression* source);
void serialize_tex_operation(char* buffer, const expression* source);
void serialize_tex_derivative(char* buffer, const expression* source);
void serialize_tex_integral(char* buffer, const expression* source);
void serialize_tex_list(char* buffer, const expression* source);
void serialize_tex_matrix(char* buffer, const expression* source);
void serialize_tex_function_structure(char* buffer, const expression* source);
void serialize_tex(char* buffer, const expression* source);
void serialize_tikz_literal(char* buffer, const expression* source);
void serialize_tikz_symbol(char* buffer, const expression* source);
void serialize_tikz_operation(char* buffer, const expression* source);
void serialize_tikz_function_structure(char* buffer, const expression* source);
void serialize_tikz(char* buffer, const expression* source);

uint8_t utoa(char* buffer, uintmax_t value) {
    
    uint8_t position = 0;
    uintmax_t divisor = 1;
    
    while (value >= (divisor *= 10));
    
    while (divisor /= 10) {
        buffer[position++] = (value - value % divisor) / divisor % 10 + '0';
    }
    
    buffer[position] = 0;
    
    return position;
    
}

uint8_t ftoa_(char* buffer, double value, bool add_parentheses) {
    
    double integer_part;
    double fractional_part;
    uint8_t position = 0;
    char temp_buffer[15];
    
    if (isnan(value)) {
        strcpy(buffer, "NAN");
        return 3;
    }
    
    if (isinf(value)) {
        strcpy(buffer, "INF");
        return 3;
    }
    
    if (are_equal(value, 0)) {
        strcpy(buffer, "0");
        return 1;
    }
    
    if (value < 0) {
        
        value = fabs(value);
        
        if (add_parentheses) {
            strcpy(buffer + position, "(");
            position++;
        }
        
        strcpy(buffer + position, "-");
        position++;
        
    }
    
    fractional_part = modf(value, &integer_part);
    position += utoa(buffer + position, (uintmax_t) (integer_part + 0.5));
    
    if (!are_equal(fractional_part, 0)) {
        
        strcpy(buffer + position, ".");
        position++;
        
        memset(temp_buffer, 0, 15);
        fractional_part = round(1e+6 * (1 + fractional_part));
        utoa(temp_buffer, (uintmax_t) (fractional_part + 0.5));
        
        strcpy(buffer + position, temp_buffer + 1);
        position += strlen(temp_buffer) - 1;
        
        for ( ; buffer[position - 1] == '0'; position--) {
            buffer[position - 1] = 0;
        }
        
        if (buffer[position - 1] == '.') {
            position--;
            buffer[position] = 0;
        }
        
    }
    
    if (buffer[0] == '(') {
        strcpy(buffer + position, ")");
        position++;
    }
    
    buffer[position] = 0;
    
    return position;
    
}

uint8_t ftoa(char* buffer, double value, bool add_parentheses) {
    
    double exponent;
    double mantissa;
    uint8_t position = 0;
    
    if (!isfinite(value) || are_equal(value, 0) || (fabs(value) > 1e-3 && fabs(value) < 1e+6)) {
        return ftoa_(buffer + position, value, add_parentheses);
    }
    
    exponent = floor(log10(fabs(value)));
    mantissa = value / pow(10, exponent);
    
    if (add_parentheses) {
        strcpy(buffer + position, "(");
        position++;
    }
    
    position += ftoa_(buffer + position, mantissa, false);
    
    strcpy(buffer + position, "E");
    position++;
    
    if (exponent > 0) {
        strcpy(buffer + position, "+");
        position++;
    }
    
    position += ftoa_(buffer + position, exponent, false);
    
    if (add_parentheses) {
        strcpy(buffer + position, ")");
        position++;
    }
    
    buffer[position] = 0;
    
    return position;
    
}

const char* get_expression_string(expression_identifier identifier) {
    
    uint8_t i;
    
    for (i = 0; keyword_identifiers[i] != EXPI_NULL; i++) {
        if (keyword_identifiers[i] == identifier) {
            if (use_abbreviations && keyword_identifiers[i + 1] == identifier) {
                return keyword_strings[i + 1];
            } else {
                return keyword_strings[i];
            }
        }
    }
    
    return "NULL";
    
}

bool serialize_value_as_fraction(numeric_value value) {
    return force_fractions || ((value.numerator * 1000) % value.denominator != 0 && value.numerator <= 100 && value.denominator <= 100);
}

void serialize_infix_literal(char* buffer, const expression* source) {
    
    numeric_value value = source->value.numeric;
    
    if (serialize_value_as_fraction(value)) {
        strcat(buffer, "(");
        if (source->sign == -1) strcat(buffer, "-");
        utoa(buffer + strlen(buffer), value.numerator);
        strcat(buffer, (use_spaces) ? " / " : "/");
        utoa(buffer + strlen(buffer), value.denominator);
        strcat(buffer, ")");
    } else {
        ftoa(buffer + strlen(buffer), (double) source->sign * value.numerator / value.denominator, true);
    }
    
}

void serialize_infix_operation(char* buffer, const expression* source) {
    
    uint8_t i;
    bool needs_parentheses = source->parent && source->identifier < source->parent->identifier && source->parent->type == EXPT_OPERATION;
    
    if (needs_parentheses) strcat(buffer, "(");
    
    for (i = 0; i < source->child_count; i++) {
        serialize_infix(buffer, source->children[i]);
        if (use_spaces && i != source->child_count - 1) strcat(buffer, " ");
        if (i != source->child_count - 1) strcat(buffer, get_expression_string(source->identifier));
        if (use_spaces && i != source->child_count - 1) strcat(buffer, " ");
    }
    
    if (needs_parentheses) strcat(buffer, ")");
    
}

void serialize_infix_function_structure(char* buffer, const expression* source) {
    
    uint8_t i;
    
    strcat(buffer, get_expression_string(source->identifier));
    strcat(buffer, "(");
    for (i = 0; i < source->child_count; i++) {
        serialize_infix(buffer, source->children[i]);
        if (i != source->child_count - 1) strcat(buffer, (use_spaces) ? ", " : ",");
    }
    strcat(buffer, ")");
    
}

void serialize_infix(char* buffer, const expression* source) {
    
    if (source == NULL) {
        strcat(buffer, "NULL");
        return;
    }
    
    if (source->identifier == EXPI_LITERAL) {
        serialize_infix_literal(buffer, source);
    } else if (source->identifier == EXPI_FLOATING) {
        ftoa(buffer + strlen(buffer), source->value.floating, true);
    } else if (source->identifier == EXPI_SYMBOL) {
        strcat(buffer, source->value.symbolic);
    } else if (source->type == EXPT_OPERATION) {
        serialize_infix_operation(buffer, source);
    } else if (source->type == EXPT_FUNCTION || source->type == EXPT_STRUCTURE) {
        serialize_infix_function_structure(buffer, source);
    }
    
}

void serialize_tex_literal(char* buffer, const expression* source) {
    
    numeric_value value = source->value.numeric;
    
    if (serialize_value_as_fraction(value)) {
        strcat(buffer, "\\frac{");
        if (source->sign == -1) strcat(buffer, "-");
        utoa(buffer + strlen(buffer), value.numerator);
        strcat(buffer, "}{");
        utoa(buffer + strlen(buffer), value.denominator);
        strcat(buffer, "}");
    } else {
        if (source->sign == -1) strcat(buffer, "\\left(-");
        ftoa(buffer + strlen(buffer), (double) value.numerator / value.denominator, false);
        if (source->sign == -1) strcat(buffer, "\\right)");
    }
    
}

void serialize_tex_symbol(char* buffer, const expression* source) {
    if (strcmp(source->value.symbolic, "pi") == 0) {
        strcat(buffer, "\\pi");
    } else {
        strcat(buffer, source->value.symbolic);
    }
}

void serialize_tex_operation(char* buffer, const expression* source) {
    
    uint8_t i;
    bool needs_brackets = source->parent && source->parent->identifier == EXPI_EXPONENTIATION;
    bool needs_parentheses = source->parent && source->identifier < source->parent->identifier && source->parent->type == EXPT_OPERATION && !needs_brackets;
    
    if (needs_parentheses) strcat(buffer, "\\left(");
    if (needs_brackets) strcat(buffer, "{");
    
    for (i = 0; i < source->child_count; i++) {
        serialize_tex(buffer, source->children[i]);
        if (i != source->child_count - 1) strcat(buffer, " ");
        if (i != source->child_count - 1) strcat(buffer, get_expression_string(source->identifier));
        if (i != source->child_count - 1) strcat(buffer, " ");
    }
    
    if (needs_brackets) strcat(buffer, "}");
    if (needs_parentheses) strcat(buffer, "\\right)");
    
}

void serialize_tex_derivative(char* buffer, const expression* source) {
    
    bool needs_brackets = source->parent && source->parent->identifier == EXPI_EXPONENTIATION;
    
    if (source->child_count != 1 && source->child_count != 2) {
        serialize_tex_function_structure(buffer, source);
        return;
    }
    
    if (needs_brackets) strcat(buffer, "{");
    strcat(buffer, "\\dfrac{\\textrm{d}}{\\textrm{d}");
    
    if (source->child_count == 1) {
        serialize_tex(buffer, get_symbol(source->children[0]));
    } else if (source->child_count == 2) {
        serialize_tex(buffer, source->children[1]);
    }
    
    strcat(buffer, "}\\left(");
    serialize_tex(buffer, source->children[0]);
    strcat(buffer, "\\right)");
    if (needs_brackets) strcat(buffer, "}");
    
}

void serialize_tex_integral(char* buffer, const expression* source) {
    
    bool needs_brackets = source->parent && source->parent->identifier == EXPI_EXPONENTIATION;
    bool needs_parentheses = source->children[0]->identifier == EXPI_ADDITION;
    
    if (needs_brackets) strcat(buffer, "{");
    strcat(buffer, "\\int");
    
    if (source->child_count > 2) {
        strcat(buffer, "_{");
        serialize_tex(buffer, source->children[source->child_count - 2]);
        strcat(buffer, "}^{");
        serialize_tex(buffer, source->children[source->child_count - 1]);
        strcat(buffer, "}");
    }

    strcat(buffer, " \\! ");
    if (needs_parentheses) strcat(buffer, "\\left(");
    serialize_tex(buffer, source->children[0]);
    if (needs_parentheses) strcat(buffer, "\\right)");
    strcat(buffer, " \\, \\textrm{d}");
    
    if (source->child_count == 2 || source->child_count == 4) {
        serialize_tex(buffer, source->children[1]);
    } else {
        serialize_tex(buffer, get_symbol(source->children[0]));
    }
    
    if (needs_brackets) strcat(buffer, "}");
    
}

void serialize_tex_list(char* buffer, const expression* source) {
    
    uint8_t i;
    
    strcat(buffer, "\\begin{bmatrix} ");
    
    for (i = 0; i < source->child_count; i++) {
        serialize_tex(buffer, source->children[i]);
        if (i != source->child_count - 1) strcat(buffer, " & ");
    }
    
    strcat(buffer, " \\end{bmatrix}");
    
}

void serialize_tex_matrix(char* buffer, const expression* source) {
    
    uint8_t i, j;
    
    strcat(buffer, "\\begin{bmatrix} ");
    
    for (i = 0; i < source->child_count; i++) {
        for (j = 0; j < source->child_count; j++) {
            serialize_tex(buffer, source->children[i]->children[j]);
            if (j != source->children[i]->child_count - 1) strcat(buffer, " & ");
        }
        if (i != source->child_count - 1) strcat(buffer, " \\\\ ");
    }
    
    strcat(buffer, " \\end{bmatrix}");
    
}

void serialize_tex_function_structure(char* buffer, const expression* source) {
    
    uint8_t i;
    
    strcat(buffer, "\\textrm{");
    strcat(buffer, get_expression_string(source->identifier));
    strcat(buffer, "} \\! \\left(");
    
    for (i = 0; i < source->child_count; i++) {
        serialize_tex(buffer, source->children[i]);
        if (i != source->child_count - 1) strcat(buffer, ", ");
    }
    
    strcat(buffer, " \\right)");
    
}

void serialize_tex(char* buffer, const expression* source) {
    
    if (source == NULL) {
        strcat(buffer, "\\textrm{NULL}");
        return;
    }
    
    if (source->identifier == EXPI_LITERAL) {
        serialize_tex_literal(buffer, source);
    } else if (source->identifier == EXPI_FLOATING) {
        ftoa(buffer + strlen(buffer), source->value.floating, false);
    } else if (source->identifier == EXPI_SYMBOL) {
        serialize_tex_symbol(buffer, source);
    } else if (source->type == EXPT_OPERATION) {
        serialize_tex_operation(buffer, source);
    } else if (source->identifier == EXPI_DERIVATIVE) {
        serialize_tex_derivative(buffer, source);
    } else if (source->identifier == EXPI_INTEGRAL) {
        serialize_tex_integral(buffer, source);
    } else if (source->identifier == EXPI_LIST) {
        serialize_tex_list(buffer, source);
    } else if (source->identifier == EXPI_MATRIX) {
        serialize_tex_matrix(buffer, source);
    } else if (source->type == EXPT_FUNCTION || source->type == EXPT_STRUCTURE) {
        serialize_tex_function_structure(buffer, source);
    }
    
}

void serialize_tikz_literal(char* buffer, const expression* source) {
    
    numeric_value value = source->value.numeric;
    
    strcat(buffer, "child{node{$");
    if (source->sign == -1) strcat(buffer, "-");

    if (serialize_value_as_fraction(value)) {
        strcat(buffer, "\\frac{");
        utoa(buffer + strlen(buffer), value.numerator);
        strcat(buffer, "}{");
        utoa(buffer + strlen(buffer), value.denominator);
        strcat(buffer, "}");
    } else {
        ftoa(buffer + strlen(buffer), (double) value.numerator / value.denominator, false);
    }

    strcat(buffer, "$}}");
    
}

void serialize_tikz_symbol(char* buffer, const expression* source) {
    
    strcat(buffer, "child{node{$");
    if (source->sign == -1) strcat(buffer, "- ");
    
    if (strcmp(source->value.symbolic, "pi") == 0) {
        strcat(buffer, "\\pi");
    } else {
        strcat(buffer, source->value.symbolic);
    }
    
    strcat(buffer, "$}}");
    
}

void serialize_tikz_operation(char* buffer, const expression* source) {
    
    uint8_t i;
    
    strcat(buffer, (source->parent) ? "child{node[operation]{$" : "\\node[operation]{$");
    if (source->sign == -1) strcat(buffer, "- ");
    strcat(buffer, (source->identifier == EXPI_EXPONENTIATION) ? "\\wedge" : get_expression_string(source->identifier));
    strcat(buffer, "$}");
    for (i = 0; i < source->child_count; i++) serialize_tikz(buffer, source->children[i]);
    if (source->parent) strcat(buffer, "}");
    
}

void serialize_tikz_function_structure(char* buffer, const expression* source) {
    
    uint8_t i;
    
    strcat(buffer, (source->parent) ? "child{node[function]{" : "\\node[function]{");
    if (source->sign == -1) strcat(buffer, "- ");
    strcat(buffer, get_expression_string(source->identifier));
    strcat(buffer, "}");
    for (i = 0; i < source->child_count; i++) serialize_tikz(buffer, source->children[i]);
    if (source->parent) strcat(buffer, "}");
    
}

void serialize_tikz(char* buffer, const expression* source) {
    
    if (source == NULL) {
        strcat(buffer, "child{node{NULL}}");
        return;
    }
    
    if (source->identifier == EXPI_LITERAL) {
        serialize_tikz_literal(buffer, source);
    } else if (source->identifier == EXPI_FLOATING) {
        ftoa(buffer + strlen(buffer), source->value.floating, false);
    } else if (source->identifier == EXPI_SYMBOL) {
        serialize_tikz_symbol(buffer, source);
    } else if (source->type == EXPT_OPERATION) {
        serialize_tikz_operation(buffer, source);
    } else if (source->type == EXPT_FUNCTION || source->type == EXPT_STRUCTURE) {
        serialize_tikz_function_structure(buffer, source);
    }
    
}

void serialize(char* buffer, const expression* source, serializer_format format) {
    
    set_parents((expression*) source);
    
    switch (format) {
        case SERF_INFIX:
            serialize_infix(buffer, source);
            break;
        case SERF_TEX:
            strcat(buffer, "\\[ ");
            serialize_tex(buffer, source);
            strcat(buffer, " \\]");
            break;
        case SERF_TIKZ:
            strcat(buffer, "\\begin{tikzpicture}");
            serialize_tikz(buffer, source);
            strcat(buffer, ";\\end{tikzpicture}");
            break;
        default:
            serialize_infix(buffer, source);
            break;
    }
    
}
