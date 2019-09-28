
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

expression* new_matrix(uint8_t rows, uint8_t colums) {
    
    uint8_t i, j;
    expression* result = new_expression(EXPT_STRUCTURE, EXPI_MATRIX, 0);
    
    for (i = 0; i < rows; i++) {
        append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 0));
        for (j = 0; j < colums; j++) {
            append_child(result->children[i], new_expression(EXPT_STRUCTURE, EXPI_LIST, 0));
        }
    }
    
    result->child_count = rows;
    
    return result;
    
}

expression* new_sub_matrix(expression* matrix, uint8_t rows, uint8_t colums) {
    
    uint8_t i, j;
    uint8_t a;
    uint8_t b;
    expression* result = new_matrix(matrix->child_count - 1, matrix->children[0]->child_count - 1);
    
    for (i = 0; i < matrix->child_count - 1; i++) {
        for (j = 0; j < matrix->children[0]->child_count -1; j++) {
            a = i;
            if (i >= rows) {
                a = i + 1;
            }
            b = i;
            if (j >= colums) {
                b = j + 1;
            }
            result->children[i]->children[j] = copy_expression(matrix->children[a]->children[b]);
        }
    }
    
    return result;
    
}

void sylvester_matrix(expression** matrix, expression* a, expression* b) {
    
    uint8_t i, j;
    expression* result;
    uint8_t a_degree = a->child_count - 1;
    uint8_t b_degree = b->child_count - 1;
    uint8_t size = a_degree + b_degree;
    
    result = new_matrix(size, size);
    
    for (i = 0; i < a_degree + 1; i++) {
        for (j = 0; j < b_degree; j++) {
            result->children[j]->children[i + j] = copy_expression(a->children[a->child_count - 1 - i]);
        }
    }
    
    for (i = 0; i < b_degree + 1; i++) {
        for (j = 0; j < a_degree; j++) {
            result->children[j + b_degree]->children[i + j] = copy_expression(b->children[b->child_count - 1 - i]);
        }
    }
    
    *matrix = result;
    
}

void gauss_determinant(expression** determinant, const expression* matrix) {
    
    uint8_t i, j;
    uint8_t factor = 1;
    uint8_t max = 0;
    expression* sub_matrix;
    expression* f;
    expression* new_row = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    expression* new_determinant = smart_alloc(1, sizeof(expression));
    expression* result;
    
    if (matrix->child_count == 1 && matrix->children[0]->child_count == 1) {
        *determinant = copy_expression(matrix->children[0]->children[0]);
        return;
    }
    
    for (i = 0; i < matrix->child_count; i++) {
        if (!expressions_are_equivalent(matrix->children[i]->children[0], new_literal(1, 0, 1), false)) {
            max = i;
            break;
        }
    }
    
    if (expressions_are_equivalent(matrix->children[max]->children[0], new_literal(1, 0, 1), false)) {
        
        *determinant = new_literal(1, 0, 1);
        return;
        
    } else {
        
        swap_expressions(matrix->children[0], matrix->children[max]);
        
        if (max != 0) {
            factor *= -1;
        }
        
        sub_matrix = new_sub_matrix(copy_expression(matrix), 0, 0);
        
        for (i = 1; i < matrix->child_count; i++) {
            
            f = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                               new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                              new_literal(-1, 1, 1),
                                              copy_expression(matrix->children[i]->children[0])),
                               copy_expression(matrix->children[0]->children[0]));
            
            replace_null_with_zero(f);
            any_expression_to_expression_recursive(f);
            
            simplify(f, true);
            
            if (expression_contains_division(f) != -1) {
                make_monic(f->children[1]);
            }
            
            new_row = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
            
            for (j = 1; j < matrix->child_count; j++) {
                append_child(new_row, new_expression(EXPT_OPERATION, EXPI_ADDITION, 2,
                                                     new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                                    copy_expression(f),
                                                                    copy_expression(matrix->children[0]->children[j])),
                                                     copy_expression(sub_matrix->children[i - 1]->children[j - 1])));
            }
            
            replace_null_with_zero(new_row);
            any_expression_to_expression_recursive(new_row);
            simplify(new_row, true);
            
            sub_matrix->children[i - 1] = copy_expression(new_row);
            
        }
        
        gauss_determinant(&new_determinant, sub_matrix);
        
        result = new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                 copy_expression(matrix->children[0]->children[0]),
                                 copy_expression(new_determinant),
                                 new_literal(factor, 1, 1));
        
        simplify(result, true);
        
    }
    
    *determinant = result;
    
}

void calculate_resultant(expression** result, expression* a, expression* b) {
    
    expression* matrix = new_expression(EXPT_STRUCTURE, EXPI_MATRIX, 0);
    expression* determinant = new_expression(EXPT_NULL, EXPI_NULL, 0);
    
    if (a->child_count == 1) {
        if (expression_is_numerical(a->children[0])) {
            if (expressions_are_identical(a->children[0], new_literal(1, 0, 1), false)) {
                *result = new_literal(1, 0, 1);
                return;
            } else {
                if (b->child_count == 0) {
                    if (expression_is_numerical(b->children[0])) {
                        if (!expressions_are_identical(b->children[0], new_literal(1, 0, 1), false)) {
                            *result = new_literal(1, 1, 1);
                            return;
                        }
                    }
                }
            }
        } else {
            if (expressions_are_identical(a->children[0], new_literal(1, 0, 1), false)) {
                *result = new_literal(1, 0, 1);
                return;
            } else {
                if (b->child_count == 0) {
                    if (expression_is_numerical(b->children[0])) {
                        if (!expressions_are_identical(b->children[0], new_literal(1, 0, 1), false)) {
                            *result = new_literal(1, 1, 1);
                            return;
                        }
                    }
                }
            }
        }
    }
    
    if (b->child_count == 1) {
        if (expression_is_numerical(b->children[0])) {
            *result = new_literal(1, 0, 1);
            return;
        }
    }
    
    sylvester_matrix(&matrix, a, b);
    gauss_determinant(&determinant, matrix);
    simplify(determinant, true);
    any_expression_to_dense_polynomial(determinant, new_symbol(EXPI_SYMBOL, "EZ"));
    
    *result = determinant;
    
}


