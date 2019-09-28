
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

bool changed = false;

void simplify_literal(expression* source);

void merge_additions_multiplications(expression* source);
uint8_t numeric_addition(expression** result, expression* a, expression* b, bool persistent);
uint8_t symbolic_addition(expression** result, expression* a, expression* b, bool persistent);
void evaluate_addition(expression* source);
void simplify_addition(expression* source);

uint8_t numeric_multiplication(expression** result, expression* a, expression* b, bool persistent);
uint8_t symbolic_multiplication(expression** result, expression* a, expression* b, bool persistent);
void evaluate_multiplication(expression* source);
uint8_t expand_multiplication(expression* source);
void simplify_multiplication(expression* source);

uint8_t expand_exponentation_base(expression* source);
uint8_t expand_exponentation_exponent(expression* source);
uint8_t numeric_exponentation(expression* source);
uint8_t symbolic_exponentation(expression* source);
return_status evaluate_exponentation(expression* source);
return_status simplify_exponentation(expression* source);

void simplify_literal(expression* source) {
    uintmax_t gcd;
    if (source->identifier != EXPI_LITERAL) return;
    gcd = euclidean_gcd(source->value.numeric.numerator, source->value.numeric.denominator);
    source->value.numeric.numerator /= gcd;
    source->value.numeric.denominator /= gcd;
}

void merge_additions_multiplications(expression* source) {
    
    uint8_t i, j;
    expression_identifier identifier;
    expression* result;
    
    identifier = source->identifier;
    
    if (identifier != EXPI_ADDITION && identifier != EXPI_MULTIPLICATION) return;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        if (source->children[i]->identifier == identifier) break; /* source has at least one child with the same identifier -> break to continue the merge process */
        if (i == source->child_count - 1) return; /* all children of source were searched an no one had the same identifier as the source -> expressions cannot be merged -> return */
    }
    
    result = new_expression(EXPT_OPERATION, identifier, 0);
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        if (source->children[i]->identifier == identifier) {
            merge_additions_multiplications(source->children[i]);
            for (j = 0; j < source->children[i]->child_count; j++) {
                append_child(result, copy_expression(source->children[i]->children[j]));
            }
        } else {
            append_child(result, copy_expression(source->children[i]));
        }
    }
    
    replace_expression(source, result);
    
    changed = true;
    
    return;
    
}

uint8_t numeric_addition(expression** result, expression* a, expression* b, bool persistent) {
    
    numeric_value a_value;
    numeric_value b_value;
    uintmax_t temp_1;
    uintmax_t temp_2;
    uintmax_t temp_3;
    
    if (a->identifier != EXPI_LITERAL || b->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
    
    a_value = a->value.numeric;
    b_value = b->value.numeric;
    
    if ((a->sign == 1 && b->sign == 1) || (a->sign == -1 && b->sign == -1)) {
        
        if (multiplication(&temp_1, a_value.numerator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
        if (multiplication(&temp_2, a_value.denominator, b_value.numerator) == RETS_ERROR) return RETS_UNCHANGED;
        if (addition(&temp_1, temp_1, temp_2) == RETS_ERROR) return RETS_UNCHANGED;
        
        if (multiplication(&temp_2, a_value.denominator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
        
        *result = new_literal(a->sign, temp_1, temp_2);
        
    } else {
        
        if (multiplication(&temp_1, a_value.numerator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
        if (multiplication(&temp_2, a_value.denominator, b_value.numerator) == RETS_ERROR) return RETS_UNCHANGED;
        if (multiplication(&temp_3, a_value.denominator, b_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
        
        if (temp_1 > temp_2) {
            *result = new_literal(a->sign, temp_1 - temp_2, temp_3);
        } else {
            *result = new_literal(b->sign, temp_2 - temp_1, temp_3);
        }
        
    }
    
    simplify_literal(*result);
    
    if (!persistent) {
        free_expression(a, false);
        free_expression(b, false);
    }
    
    changed = true;
    
    return RETS_CHANGED;
    
}

uint8_t symbolic_addition(expression** result, expression* a, expression* b, bool persistent) {
    
    expression* a_temp;
    expression* b_temp;
    expression* a_factor;
    expression* b_factor;
    expression* temp;
    
    if (a->identifier == EXPI_MULTIPLICATION && a->children[0]->identifier == EXPI_LITERAL) {
        a_factor = copy_expression(a->children[0]);
        a_temp = copy_expression(a);
        remove_child_at_index(a_temp, 0);
        simplify(a_temp, false);
    } else {
        a_temp = copy_expression(a);
        a_factor = new_literal(1, 1, 1);
    }
    
    if (b->identifier == EXPI_MULTIPLICATION && b->children[0]->identifier == EXPI_LITERAL) {
        b_factor = copy_expression(b->children[0]);
        b_temp = copy_expression(b);
        remove_child_at_index(b_temp, 0);
        simplify(b_temp, false);
    } else {
        b_temp = copy_expression(b);
        b_factor = new_literal(1, 1, 1);
    }
    
    if (expressions_are_identical(a_temp, b_temp, true)) {
        numeric_addition(&temp, a_factor, b_factor, false);
        *result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                 temp,
                                 a_temp);
        free_expression(b_temp, false);
        changed = true;
        return RETS_CHANGED;
    } else {
        free_expression(a_temp, false);
        free_expression(b_temp, false);
        free_expression(a_factor, false);
        free_expression(b_factor, false);
        return RETS_UNCHANGED;
    }
    
}

void evaluate_addition(expression* source) {
    
    uint8_t i, j;
    expression* temp_result;
    expression* result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count; i++) {
        
        if (source->children[i] == NULL) continue;
        
        if (source->children[i]->identifier == EXPI_LITERAL && literal_to_double(source->children[i]) == 0) {
            free_expression(source->children[i], false);
            source->children[i] = NULL;
        }
        
        for (j = i + 1; j < source->child_count; j++) {
            
            if (source->children[i] == NULL) continue;
            if (source->children[j] == NULL) continue;
            
            if (numeric_addition(&temp_result, source->children[i], source->children[j], true) == RETS_CHANGED) {
                replace_expression(source->children[i], temp_result);
                free_expression(source->children[j], false);
                source->children[j] = NULL;
                continue;
            }
            
            if (symbolic_addition(&temp_result, source->children[i], source->children[j], true) == RETS_CHANGED) {
                replace_expression(source->children[i], temp_result);
                free_expression(source->children[j], false);
                source->children[j] = NULL;
                continue;
            }
            
        }
        
    }
    
    remove_null_children(source);
    
    switch (source->child_count) {
        case 0: result = new_literal(1, 0, 1); break;
        case 1: result = copy_expression(source->children[0]); break;
        default: result = copy_expression(source); break;
    }
    
    replace_expression(source, result);
    
    return;
    
}

void simplify_addition(expression* source) {
    
    merge_additions_multiplications(source);
    evaluate_addition(source);
    if (source->child_count > 0) order_children(source);
    
    return;
    
}

void simplify_subtraction(expression* source) {
    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                              copy_expression(source->children[0]),
                                              new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                             new_literal(-1, 1, 1),
                                                             copy_expression(source->children[1]))));
    changed = true;
    return;
}

uint8_t numeric_multiplication(expression** result, expression* a, expression* b, bool persistent) {
    
    numeric_value a_value;
    numeric_value b_value;
    uintmax_t temp_1;
    uintmax_t temp_2;
    
    if (a->identifier != EXPI_LITERAL || b->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
    
    a_value = a->value.numeric;
    b_value = b->value.numeric;
    
    if (multiplication(&temp_1, a_value.numerator, b_value.numerator) == RETS_ERROR) return RETS_UNCHANGED;
    if (multiplication(&temp_2, b_value.denominator, a_value.denominator) == RETS_ERROR) return RETS_UNCHANGED;
    
    *result = new_literal(a->sign * b->sign, temp_1, temp_2);
    simplify_literal(*result);
    
    if (!persistent) {
        free_expression(a, false);
        free_expression(b, false);
    }
    
    changed = true;
    
    return RETS_CHANGED;
    
}

uint8_t symbolic_multiplication(expression** result, expression* a, expression* b, bool persistent) {
    
    expression* a_temp;
    expression* b_temp;
    expression* a_exponent;
    expression* b_exponent;
    expression* temp;
    
    if (a->identifier == EXPI_EXPONENTATION && a->children[1]->identifier == EXPI_LITERAL) {
        a_temp = copy_expression(a->children[0]);
        a_exponent = copy_expression(a->children[1]);
    } else {
        a_temp = copy_expression(a);
        a_exponent = new_literal(1, 1, 1);
    }
    
    if (b->identifier == EXPI_EXPONENTATION && b->children[1]->identifier == EXPI_LITERAL) {
        b_temp = copy_expression(b->children[0]);
        b_exponent = copy_expression(b->children[1]);
    } else {
        b_temp = copy_expression(b);
        b_exponent = new_literal(1, 1, 1);
    }
    
    if (expressions_are_identical(a_temp, b_temp, true)) {
        numeric_addition(&temp, a_exponent, b_exponent, false);
        *result = new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                 a_temp,
                                 temp);
        free_expression(b_temp, false);
        changed = true;
        return RETS_CHANGED;
    } else {
        free_expression(a_temp, false);
        free_expression(b_temp, false);
        free_expression(a_exponent, false);
        free_expression(b_exponent, false);
        return RETS_UNCHANGED;
    }
    
}

void evaluate_multiplication(expression* source) {
    
    uint8_t i, j;
    expression* temp_result;
    expression* result;
    
    for (i = 0; i < source->child_count; i++) {
        
        if (source->children[i] == NULL) continue;
        
        if (source->children[i]->identifier == EXPI_LITERAL && literal_to_double(source->children[i]) == 0) {
            replace_expression(source, new_literal(1, 0, 1));
            return;
        }
        
        if (source->children[i]->identifier == EXPI_LITERAL && literal_to_double(source->children[i]) == 1) {
            free_expression(source->children[i], false);
            source->children[i] = NULL;
            continue;
        }
        
        for (j = i + 1; j < source->child_count; j++) {
            
            if (source->children[i] == NULL) continue;
            if (source->children[j] == NULL) continue;
            
            if (numeric_multiplication(&temp_result, source->children[i], source->children[j], true) == RETS_CHANGED) {
                replace_expression(source->children[i], temp_result);
                free_expression(source->children[j], false);
                source->children[j] = NULL;
                continue;
            }
            
            if (symbolic_multiplication(&temp_result, source->children[i], source->children[j], true) == RETS_CHANGED) {
                replace_expression(source->children[i], temp_result);
                free_expression(source->children[j], false);
                source->children[j] = NULL;
                continue;
            }
            
        }
        
    }
    
    remove_null_children(source);
    
    switch (source->child_count) {
        case 0: result = new_literal(1, 1, 1); break;
        case 1: result = copy_expression(source->children[0]); break;
        default: result = copy_expression(source); break;
    }
    
    replace_expression(source, result);
    
    return;
    
}

void expand_multiplication_addition_factors(expression* source) {
    
    uint8_t i, j, k, l;
    expression* result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count - 1; i++) {
        for (j = 0; j < source->children[i]->child_count; j++) {
            for (k = i + 1; k < source->child_count; k++) {
                for (l = 0; l < source->children[k]->child_count; l++) {
                    append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                        copy_expression(source->children[i]->children[j]),
                                                        copy_expression(source->children[k]->children[l])));
                }
            }
        }
    }
    
    simplify(result, true);
    replace_expression(source, result);
    
    return;
    
}

uint8_t expand_multiplication(expression* source) {
    
    uint8_t i;
    expression* single_factors;
    expression* addition_factors;
    expression* result;
    
    if (source->child_count == 0) return RETS_UNCHANGED;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == NULL) continue;
        if (source->children[i]->identifier == EXPI_ADDITION) break; /* source has at least one child being an additon -> break to continue the expanding process */
        if (i == source->child_count - 1) return RETS_UNCHANGED; /* all children of source were searched an no one had an addion as its child -> multiplication cannot be expanded -> return */
    }
    
    single_factors = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
    addition_factors = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
    result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count; i++) {
        
        if (source->children[i] == NULL) continue;
        
        if (source->children[i]->identifier == EXPI_ADDITION) {
            append_child(addition_factors, copy_expression(source->children[i]));
        } else {
            append_child(single_factors, copy_expression(source->children[i]));
        }
        
    }
    
    if (addition_factors->child_count > 1) {
        expand_multiplication_addition_factors(addition_factors);
    } else {
        replace_expression(addition_factors, copy_expression(addition_factors->children[0]));
    }
    
    for (i = 0; i < addition_factors->child_count; i++) {
        append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                            copy_expression(single_factors),
                                            copy_expression(addition_factors->children[i])));
    }
    
    simplify(result, true);
    replace_expression(source, result);
    
    free_expression(single_factors, false);
    free_expression(addition_factors, false);
    
    changed = true;
    
    return RETS_CHANGED;
    
}

void simplify_multiplication(expression* source) {
    
    merge_additions_multiplications(source);
    if (expand_multiplication(source) == RETS_CHANGED) return;
    evaluate_multiplication(source);
    order_children(source);
    
    return;
    
}

void simplify_division(expression* source) {
    
    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                              copy_expression(source->children[0]),
                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                             copy_expression(source->children[1]),
                                                             new_literal(-1, 1, 1))));
    
    changed = true;
    
    return;
    
}

void merge_nested_exponentations(expression* source) {
    
    expression* result;
    
    if (!expression_is_constant(source)) return;
    
    if (source->children[0]->identifier == EXPI_EXPONENTATION) {
        result = new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                copy_expression(source->children[0]->children[0]),
                                new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                               copy_expression(source->children[0]->children[1]),
                                               copy_expression(source->children[1])));
        replace_expression(source, result);
        simplify(source->children[1], true);
        changed = true;
    }
    
    return;
    
}

uint8_t expand_exponentation_base(expression* source) {
    
    uint8_t i;
    expression* base = source->children[0];
    expression* exponent = source->children[1];
    expression* result;
    uintmax_t* coefficients;
    
    if (base->identifier == EXPI_ADDITION && base->child_count == 2 && exponent->identifier == EXPI_LITERAL && exponent->value.numeric.numerator <= 10 && exponent->value.numeric.denominator == 1 && exponent->sign == 1) {
        
        result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
        coefficients = binomial_coefficients(exponent->value.numeric.numerator);
        
        for (i = 0; i <= exponent->value.numeric.numerator; i++) {
            append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                                new_literal(1, coefficients[i], 1),
                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                               copy_expression(base->children[0]),
                                                               new_literal(1, exponent->value.numeric.numerator - i, 1)),
                                                new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                               copy_expression(base->children[1]),
                                                               new_literal(1, i, 1))));
        }
        
        smart_free(coefficients);
        replace_expression(source, result);
        
        changed = true;
        
        return RETS_CHANGED;
        
    } else if (base->identifier == EXPI_MULTIPLICATION) {
        
        result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
        
        for (i = 0; i < source->children[0]->child_count; i++) {
            append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                copy_expression(source->children[0]->children[i]),
                                                copy_expression(source->children[1])));
        }
        
        replace_expression(source, result);
        
        changed = true;
        
        return RETS_CHANGED;
        
    }
    
    return RETS_UNCHANGED;
    
}

uint8_t expand_exponentation_exponent(expression* source) {
    
    uint8_t i;
    expression* result;
    
    if (source->children[1]->identifier != EXPI_ADDITION) return RETS_UNCHANGED;
    
    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
    
    for (i = 0; i < source->children[1]->child_count; i++) {
        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                            copy_expression(source->children[0]),
                                            copy_expression(source->children[1]->children[i])));
    }
    
    replace_expression(source, result);
    
    changed = true;
    
    return RETS_CHANGED;
    
}

uint8_t remove_exponentation_identities(expression* source) {
    
    expression* base = source->children[0];
    expression* exponent = source->children[1];
    
    if (exponent->identifier == EXPI_LITERAL && literal_to_double(exponent) == 0) {
        replace_expression(source, new_literal(1, 1, 1));
        return RETS_CHANGED;
    }
    
    if (exponent->identifier == EXPI_LITERAL && literal_to_double(exponent) == 1) {
        replace_expression(source, copy_expression(base));
        return RETS_CHANGED;
    }
    
    if (base->identifier == EXPI_LITERAL && literal_to_double(base) == 0) {
        replace_expression(source, new_literal(1, 0, 1));
        return RETS_CHANGED;
    }
    
    if (base->identifier == EXPI_LITERAL && literal_to_double(base) == 1) {
        replace_expression(source, new_literal(1, 1, 1));
        return RETS_CHANGED;
    }
    
    if (base->identifier == EXPI_LITERAL && literal_to_double(base) == -1 && exponent->identifier == EXPI_LITERAL && exponent->value.numeric.numerator % 2 == 1 && exponent->value.numeric.denominator == 1) {
        replace_expression(source, new_literal(-1, 1, 1));
        return RETS_CHANGED;
    }
    
    return RETS_UNCHANGED;
    
}

uint8_t numeric_exponentation(expression* source) {
    
    expression* base = source->children[0];
    expression* exponent = source->children[1];
    expression* base_result;
    expression* factor;
    expression* result;
    
    uintmax_t temp;
    
    if (base->identifier != EXPI_LITERAL || exponent->identifier != EXPI_LITERAL) return RETS_UNCHANGED;
    
    base_result = new_literal(1, 0, 1);
    factor = new_literal(1, 1, 1);
    
    simplify_literal(base);
    simplify_literal(exponent);
    
    if (literal_to_double(base) == -1 && exponent->value.numeric.denominator == 2) {
        
        if ((exponent->value.numeric.numerator - 1) % 4 == 0) {
            replace_expression(source, new_symbol(EXPI_SYMBOL, "i"));
        } else {
            replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                      new_literal(-1, 1, 1),
                                                      new_symbol(EXPI_SYMBOL, "i")));
        }
        
        return RETS_CHANGED;
        
    }
    
    if (exponent->sign == -1) {
        temp = base->value.numeric.numerator;
        base->value.numeric.numerator = base->value.numeric.denominator;
        base->value.numeric.denominator = temp;
        exponent->sign = 1;
    }
    
    if (int_power(&base_result->value.numeric.numerator, base->value.numeric.numerator, exponent->value.numeric.numerator) == RETS_ERROR) return RETS_UNCHANGED;
    if (int_power(&base_result->value.numeric.denominator, base->value.numeric.denominator, exponent->value.numeric.numerator) == RETS_ERROR) return RETS_UNCHANGED;
    
    int_root(&factor->value.numeric.numerator, &base_result->value.numeric.numerator, base_result->value.numeric.numerator, exponent->value.numeric.denominator);
    int_root(&factor->value.numeric.denominator, &base_result->value.numeric.denominator, base_result->value.numeric.denominator, exponent->value.numeric.denominator);
    
    result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 0);
    
    if (base->sign == -1 && exponent->value.numeric.numerator % 2 == 1) {
        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                            new_literal(-1, 1, 1),
                                            copy_expression(exponent)));
//        symbolic_exponentation(result->children[result->child_count - 1]);
    }
    
    if (literal_to_double(factor) == literal_to_double(base_result)) {
        append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                            base_result,
                                            new_literal(1, exponent->value.numeric.numerator + exponent->value.numeric.denominator, exponent->value.numeric.denominator)));
        
        remove_exponentation_identities(result->children[result->child_count - 1]);
    } else {
        if (literal_to_double(factor) != 1) append_child(result, factor);
        if (literal_to_double(base_result) != 1) append_child(result, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                                     base_result,
                                                                                     copy_expression(exponent)));
    }
    
    simplify_multiplication(result);
    
    replace_expression(source, result);
    
    return RETS_CHANGED;
    
}

uint8_t symbolic_exponentation(expression* source) {
    
    expression* base = source->children[0];
    expression* exponent = source->children[1];
    
    if (expressions_are_identical(base, new_symbol(EXPI_SYMBOL, "i"), false) && exponent->identifier == EXPI_LITERAL) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                  new_literal(-1, 1, 1),
                                                  new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                 new_literal(1, 1, 2),
                                                                 copy_expression(exponent))));
        simplify(source, true);
        return RETS_CHANGED;
    }
    
    return RETS_UNCHANGED;
    
}

return_status exponentation_remove_logarithms(expression* source) {
    
    uint8_t i;
    expression* base = source->children[0];
    expression* exponent = source->children[1];
    expression* temp;
    
    if (exponent->identifier == EXPI_LN && expressions_are_identical(base, new_symbol(EXPI_SYMBOL, "e"), false)) {
        replace_expression(source, copy_expression(source->children[1]->children[0]));
        return RETS_CHANGED;
    }
    
    if (exponent->identifier == EXPI_LOG && exponent->child_count == 1 && expressions_are_identical(base, new_literal(1, 10, 1), false)) {
        replace_expression(source, copy_expression(source->children[1]->children[0]));
        return RETS_CHANGED;
    }
    
    if (exponent->identifier == EXPI_LOG && exponent->child_count == 2 && expressions_are_identical(base, exponent->children[1], true)) {
        replace_expression(source, copy_expression(source->children[1]->children[0]));
        return RETS_CHANGED;
    }
    
    if (exponent->identifier == EXPI_MULTIPLICATION) {
        for (i = 0; i < exponent->child_count; i++) {
            if (exponent->children[i]->identifier == EXPI_LN || exponent->children[i]->identifier == EXPI_LOG) {
                
                temp = new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                      copy_expression(base),
                                      copy_expression(exponent->children[i]));
                
                if (exponentation_remove_logarithms(temp) == RETS_CHANGED) {
                    remove_child_at_index(exponent, i);
                    replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                              temp,
                                                              copy_expression(exponent)));
                    simplify(source, true);
                    
                    return RETS_CHANGED;
                    
                }
                
                smart_free(temp);
                
            }
        }
    }
    
    return RETS_UNCHANGED;
    
}

return_status evaluate_exponentation(expression* source) {
    
    if (source->children[0]->identifier == EXPI_LITERAL && source->children[0]->value.numeric.numerator == 0 && source->children[1]->sign == -1) {
        return set_error(ERRD_MATH, ERRI_UNDEFINED_VALUE, "");
    }
    
    if (remove_exponentation_identities(source) == RETS_CHANGED) return RETS_SUCCESS;
    if (numeric_exponentation(source) == RETS_CHANGED) return RETS_SUCCESS;
    symbolic_exponentation(source);
    
    return RETS_SUCCESS;
    
}

return_status simplify_exponentation(expression* source) {
    
    merge_nested_exponentations(source);
    if (expand_exponentation_base(source) == RETS_CHANGED) return RETS_SUCCESS;
    if (expand_exponentation_exponent(source) == RETS_CHANGED) return RETS_SUCCESS;
    if (exponentation_remove_logarithms(source) == RETS_CHANGED) return RETS_SUCCESS;
    ERROR_CHECK(evaluate_exponentation(source));
    
    return RETS_SUCCESS;
    
}

void simplify_abs(expression* source) {
    
    expression* result;
    
    if (source->child_count == 1) {
        if (source->children[0]->identifier == EXPI_LITERAL || symbol_is_constant(source->children[0])) {
            result = copy_expression(source->children[0]);
            result->sign = 1;
            replace_expression(source, result);
            changed = true;
        }
    } else if (source->children[0]->identifier == EXPI_MULTIPLICATION && source->children[0]->child_count == 2 && source->children[0]->children[0]->sign == -1 && symbol_is_constant(source->children[0]->children[1])) {
        result = copy_expression(source->children[0]);
        result->children[0]->sign = 1;
        replace_expression(source, result);
        changed = true;
    }
    
    return;
    
}

uint8_t evaluate_logarithm(expression** result, expression* value, expression* base) {
    
    if (expressions_are_identical(value, new_literal(1, 0, 1), false)) {
        return set_error(ERRD_MATH, ERRI_UNDEFINED_VALUE, "");
    }
    
    if (expressions_are_identical(value, base, true)) {
        *result = new_literal(1, 1, 1);
        return RETS_SUCCESS;
    }
    
    if (expressions_are_identical(base, new_symbol(EXPI_SYMBOL, "e"), false)) {
        *result = new_expression(EXPT_FUNCTION, EXPI_LN, 1, copy_expression(value));
    } else if (expressions_are_identical(base, new_literal(1, 10, 1), false)) {
        *result = new_expression(EXPT_FUNCTION, EXPI_LOG, 1, copy_expression(value));
    } else {
        *result = new_expression(EXPT_FUNCTION, EXPI_LOG, 2,
                                 copy_expression(value),
                                 copy_expression(base));
    }
    
    return RETS_SUCCESS;
    
}

uint8_t expand_logarithm(expression* source) {
    
    uint8_t i;
    expression* factors;
    expression* result;
    
    if (source->identifier != EXPI_LN || source->identifier != EXPI_LN) return RETS_UNCHANGED;
    
    if (source->children[0]->identifier == EXPI_MULTIPLICATION) {
        
        result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
        
        for (i = 0; i < source->children[0]->child_count; i++) {
            append_child(result, new_expression(EXPT_FUNCTION, source->identifier, 1, copy_expression(source->children[0]->children[i])));
            if (source->child_count == 2) {
                append_child(result->children[i], copy_expression(source->children[1]));
            }
        }
        
    } else if (source->children[0]->identifier == EXPI_EXPONENTATION && expression_is_constant(source->children[0]->children[0]) && source->children[0]->children[0]->sign == 1) {
        
        result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                copy_expression(source->children[0]->children[1]),
                                copy_expression(source));
        
        replace_expression(result->children[1]->children[0], copy_expression(result->children[1]->children[0]->children[0]));
        
    } else if (source->children[0]->identifier == EXPI_LITERAL && source->children[0]->value.numeric.denominator == 1) {
        
        result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
        
        factors = prime_factors(source->children[0]->value.numeric.numerator);
        
        if (factors->child_count == 1 && factors->children[0]->children[1]->value.numeric.numerator == 1) return RETS_CHANGED;
        
        for (i = 0; i < factors->child_count; i++) {
            append_child(result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                copy_expression(factors->children[i]->children[1]),
                                                new_expression(EXPT_FUNCTION, source->identifier, 1, copy_expression(factors->children[i]->children[0]))));
        }
        
        free_expression(factors, false);
        
    } else {
        return RETS_UNCHANGED;
    }
    
    replace_expression(source, result);
    
    changed = true;
    
    return RETS_CHANGED;
    
}

uint8_t simplify_logarithm(expression* source) {
    
    expression* value;
    expression* base;
    expression* result;
    
    if (source->identifier == EXPI_LN && source->child_count == 1) {
        base = new_symbol(EXPI_SYMBOL, "e");
    } else if (source->identifier == EXPI_LOG && source->child_count == 1) {
        base = new_literal(1, 10, 1);
    } else if (source->identifier == EXPI_LOG && source->child_count == 2) {
        base = copy_expression(source->children[1]);
    } else {
        return set_error(ERRD_SYNTAX, ERRI_ARGUMENTS, "");
    }
    
    value = copy_expression(source->children[0]);
    
    ERROR_CHECK(evaluate_logarithm(&result, value, base));
    
    if (expand_logarithm(result) == RETS_CHANGED) {
        replace_expression(source, result);
        return RETS_CHANGED;
    } else {
        replace_expression(source, result);
        return RETS_SUCCESS;
    }
    
}

void simplify_sin(expression* source) {
    
    if (expressions_are_equivalent(source->children[0], new_literal(1, 0, 1), false)) {
        replace_expression(source, new_literal(1, 0, 1));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 6),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_literal(1, 1, 2));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 4),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                 new_literal(1, 2, 1),
                                                                 new_literal(1, 1, 2)),
                                                  new_literal(1, 1, 2)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 3),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                 new_literal(1, 3, 1),
                                                                 new_literal(1, 1, 2)),
                                                  new_literal(1, 1, 2)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 2),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_literal(1, 1, 1));
    } else {
        return;
    }
    
    changed = true;
    
}

void simplify_cos(expression* source) {
    
    if (expressions_are_equivalent(source->children[0], new_literal(1, 0, 1), false)) {
        replace_expression(source, new_literal(1, 1, 1));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 6),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                 new_literal(1, 3, 1),
                                                                 new_literal(1, 1, 2)),
                                                  new_literal(1, 1, 2)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 4),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                 new_literal(1, 2, 1),
                                                                 new_literal(1, 1, 2)),
                                                  new_literal(1, 1, 2)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 3),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_literal(1, 1, 2));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 2),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_literal(1, 0, 1));
    } else {
        return;
    }
    
    changed = true;
    
}

return_status simplify_tan(expression* source) {
    
    if (expressions_are_equivalent(source->children[0], new_literal(1, 0, 1), false)) {
        replace_expression(source, new_literal(1, 0, 1));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 6),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                 new_literal(1, 3, 1),
                                                                 new_literal(1, 1, 2)),
                                                  new_literal(1, 1, 3)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 4),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_literal(1, 1, 1));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 3),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                  new_literal(1, 3, 1),
                                                  new_literal(1, 1, 2)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_literal(1, 1, 2),
                                                                              new_symbol(EXPI_SYMBOL, "pi")), false)) {
        return set_error(ERRD_MATH, ERRI_UNDEFINED_VALUE, "tan");
    } else {
        return RETS_SUCCESS;
    }
    
    changed = true;
    
    return RETS_SUCCESS;
    
}

void simplify_arcsin(expression* source) {
    
    if (expressions_are_identical(source->children[0], new_literal(1, 0, 1), false)) {
        replace_expression(source, new_literal(1, 0, 1));
    } else if (expressions_are_identical(source->children[0], new_literal(1, 1, 2), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 6)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                                             new_literal(1, 2, 1),
                                                                                             new_literal(1, 1, 2)),
                                                                              new_literal(1, 1, 2)), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 4)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                                             new_literal(1, 3, 1),
                                                                                             new_literal(1, 1, 2)),
                                                                              new_literal(1, 1, 2)), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 3)));
    } else if (expressions_are_identical(source->children[0], new_literal(1, 1, 1), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 2)));
    } else {
        return;
    }
    
    changed = true;
    
}

void simplify_arccos(expression* source) {
    
    if (expressions_are_identical(source->children[0], new_literal(1, 1, 1), false)) {
        replace_expression(source, new_literal(1, 1, 1));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                                             new_literal(1, 3, 1),
                                                                                             new_literal(1, 1, 2)),
                                                                              new_literal(1, 1, 2)), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 6)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                                             new_literal(1, 2, 1),
                                                                                             new_literal(1, 1, 2)),
                                                                              new_literal(1, 1, 2)), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 4)));
    } else if (expressions_are_identical(source->children[0], new_literal(1, 1, 2), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 3)));
    } else if (expressions_are_identical(source->children[0], new_literal(1, 0, 1), false)) {
        replace_expression(source, new_literal(1, 0, 1));
    } else {
        return;
    }
    
    changed = true;
    
}

void simplify_arctan(expression* source) {
    
    if (expressions_are_identical(source->children[0], new_literal(1, 0, 1), false)) {
        replace_expression(source, new_literal(1, 0, 1));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                              new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                                             new_literal(1, 3, 1),
                                                                                             new_literal(1, 1, 2)),
                                                                              new_literal(1, 1, 3)), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 6)));
    } else if (expressions_are_identical(source->children[0], new_literal(1, 1, 1), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 4)));
    } else if (expressions_are_equivalent(source->children[0], new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                                              new_literal(1, 3, 1),
                                                                              new_literal(1, 1, 2)), false)) {
        replace_expression(source, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                  new_symbol(EXPI_SYMBOL, "pi"),
                                                  new_literal(1, 1, 3)));
    } else {
        return;
    }
    
    changed = true;
    
}

uint8_t simplify(expression* source, bool recursive) {
    
    uint8_t i;
    
    changed = false;
    
    for (i = 0; i < source->child_count && recursive; i++) {
        if (source->children[i] == NULL) continue;
        ERROR_CHECK(simplify(source->children[i], true));
    }
    
    any_expression_to_expression(source);
    
    switch (source->identifier) {
        case EXPI_LITERAL: simplify_literal(source); break;
        case EXPI_SYMBOL: break;
        case EXPI_VARIABLE: break;
        case EXPI_ADDITION: simplify_addition(source); break;
        case EXPI_SUBTRACTION: simplify_subtraction(source); break;
        case EXPI_MULTIPLICATION: simplify_multiplication(source); break;
        case EXPI_DIVISION: simplify_division(source); break;
        case EXPI_EXPONENTATION: ERROR_CHECK(simplify_exponentation(source)); break;
        case EXPI_ABS: simplify_abs(source); break;
        case EXPI_LN:
        case EXPI_LOG: ERROR_CHECK(simplify_logarithm(source)); break;
        case EXPI_SIN: simplify_sin(source); break;
        case EXPI_COS: simplify_cos(source); break;
        case EXPI_TAN: ERROR_CHECK(simplify_tan(source)); break;
        case EXPI_ARCSIN: simplify_arcsin(source); break;
        case EXPI_ARCCOS: simplify_arccos(source); break;
        case EXPI_ARCTAN: simplify_arctan(source); break;
        case EXPI_POLYNOMIAL_SPARSE: break;
        case EXPI_POLYNOMIAL_DENSE: break;
        case EXPI_LIST: break;
        case EXPI_MATRIX: break;
        case EXPI_EXTENSION: break;
        default: break;
    }
    
    if (changed) ERROR_CHECK(simplify(source, true));
    
    return RETS_SUCCESS;
    
}

void approximate_addition(expression* source) {
    
    uint8_t i;
    double result = 0;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->identifier == EXPI_LITERAL) {
            result += literal_to_double(source->children[i]);
            free_expression(source->children[i], false);
            source->children[i] = NULL;
        }
    }
    
    remove_null_children(source);
    
    if (source->child_count == 0) {
        replace_expression(source, double_to_literal(result));
    } else {
        append_child(source, double_to_literal(result));
    }
    
    return;
    
}

void approximate_multiplication(expression* source) {
    
    uint8_t i;
    double result = 1;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i]->identifier == EXPI_LITERAL) {
            result *= literal_to_double(source->children[i]);
            free_expression(source->children[i], false);
            source->children[i] = NULL;
        }
    }
    
    remove_null_children(source);
    
    if (source->child_count == 0) {
        replace_expression(source, double_to_literal(result));
    } else {
        append_child(source, double_to_literal(result));
    }
    
    return;
    
}

void approximate_exponentation(expression* source) {
    
    double result;
    
    if (source->children[0]->identifier != EXPI_LITERAL || source->children[1]->identifier != EXPI_LITERAL) return;
    
    result = pow(literal_to_double(source->children[0]), literal_to_double(source->children[1]));
    replace_expression(source,  double_to_literal(result));
    
    return;
    
}

void approximate_ln(expression* source) {
    
    double result;
    
    if (source->children[0]->identifier != EXPI_LITERAL) return;
    
    result = log(literal_to_double(source->children[0]));
    replace_expression(source,  double_to_literal(result));
    
    return;
    
}

void approximate_log(expression* source) {
    
    double result;
    
    if (source->children[0]->identifier != EXPI_LITERAL) return;
    
    result = log10(literal_to_double(source->children[0]));
    
    if (source->child_count == 2) {
        if (source->children[1]->identifier != EXPI_LITERAL) return;
        result /= log10(literal_to_double(source->children[1]));
    }
    
    replace_expression(source, double_to_literal(result));
    
    return;
    
}

void approximate_trigonometric(expression* source) {
    
    double result;
    
    if (source->children[0]->identifier != EXPI_LITERAL) return;
    
    switch (source->identifier) {
        case EXPI_SIN: result = sin(literal_to_double(source->children[0])); break;
        case EXPI_COS: result = cos(literal_to_double(source->children[0])); break;
        case EXPI_TAN: result = tan(literal_to_double(source->children[0])); break;
        case EXPI_ARCSIN: result = asin(literal_to_double(source->children[0])); break;
        case EXPI_ARCCOS: result = acos(literal_to_double(source->children[0])); break;
        case EXPI_ARCTAN: result = atan(literal_to_double(source->children[0])); break;
        default: return;
    }
    
    replace_expression(source, double_to_literal(result));
    
    return;
    
}

void approximate(expression* source) {
    
    uint8_t i;
    
    for (i = 0; i < source->child_count; i++) {
        if (source->children[i] == 0) continue;
        approximate(source->children[i]);
    }
    
    if (expressions_are_identical(source, new_symbol(EXPI_SYMBOL, "pi"), false)) {
        replace_expression(source, double_to_literal(M_PI));
    } else if (expressions_are_identical(source, new_symbol(EXPI_SYMBOL, "e"), false)) {
        replace_expression(source, double_to_literal(M_E));
    }
    
    switch (source->identifier) {
        case EXPI_ADDITION: approximate_addition(source); break;
        case EXPI_MULTIPLICATION: approximate_multiplication(source); break;
        case EXPI_EXPONENTATION: approximate_exponentation(source); break;
        case EXPI_LN: approximate_ln(source); break;
        case EXPI_LOG: approximate_log(source); break;
        case EXPI_SIN:
        case EXPI_COS:
        case EXPI_TAN:
        case EXPI_ARCSIN:
        case EXPI_ARCCOS:
        case EXPI_ARCTAN: approximate_trigonometric(source); break;
        default: break;
    }
    
}
