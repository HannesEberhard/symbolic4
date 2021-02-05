
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

expression* new_matrix(uint8_t rows, uint8_t columns) {
    
    uint8_t i, j;
    expression* result = new_expression(EXPT_STRUCTURE, EXPI_MATRIX, 0);
    
    for (i = 0; i < rows; i++) {
        append_child(result, new_expression(EXPT_STRUCTURE, EXPI_LIST, 0));
        for (j = 0; j < columns; j++) {
            append_child(result->children[i], NULL);
        }
    }
    
    result->child_count = rows;
    
    return result;
    
}

expression* new_sub_matrix(const expression* source, uint8_t row, uint8_t column) {
    
    uint8_t i, j;
    uint8_t a = 0;
    uint8_t b;
    expression* result = new_matrix(source->child_count - 1, source->children[0]->child_count - 1);
    
    for (i = 0; i < source->child_count; i++) {
        if (i == row) continue;
        b = 0;
        for (j = 0; j < source->children[0]->child_count; j++) {
            if (j == column) continue;
            result->children[a]->children[b] = copy_expression(source->children[i]->children[j]);
            b++;
        }
        a++;
    }
    
    return result;
    
}

expression* new_sylvester_matrix(const expression* a, const expression* b, const expression* variable) {
    
    uint8_t i, j;
    uint8_t a_degree;
    uint8_t b_degree;
    expression* a_temp = copy_expression(a);
    expression* b_temp = copy_expression(b);
    expression* result;
    
    any_expression_to_dense_polynomial(a_temp, variable);
    any_expression_to_dense_polynomial(b_temp, variable);
    
    a_degree = a_temp->children[1]->child_count - 1;
    b_degree = b_temp->children[1]->child_count - 1;
    
    result = new_matrix(a_degree + b_degree, a_degree + b_degree);
    
    for (i = 0; i < a_degree + 1; i++) {
        for (j = 0; j < b_degree; j++) {
            result->children[j]->children[i + j] = copy_expression(a_temp->children[1]->children[a_temp->children[1]->child_count - i - 1]);
        }
    }
    
    for (i = 0; i < b_degree + 1; i++) {
        for (j = 0; j < a_degree; j++) {
            result->children[b_degree + j]->children[i + j] = copy_expression(b_temp->children[1]->children[b_temp->children[1]->child_count - i - 1]);
        }
    }
    
//    replace_null_with_zero(result);
    free_expressions(2, a_temp, b_temp);
    
    return result;
    
}

void determinant(expression** result, const expression* source) {
    
    uint8_t i;
    int8_t sign = 1;
    expression* sub_matrix;
    expression* sub_matrix_determinant;
    
    if (source->child_count == 1) {
        *result = copy_expression(source->children[0]->children[0]);
        return;
    }
    
    *result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source->child_count; i++) {
        sub_matrix = new_sub_matrix(source, 0, i);
        determinant(&sub_matrix_determinant, sub_matrix);
        append_child(*result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 3,
                                             new_literal(sign, 1, 1),
                                             copy_expression(source->children[0]->children[i]),
                                             sub_matrix_determinant));
        sign = -sign;
        free_expression(sub_matrix, false);
    }
    
    simplify(*result, true);
    
}

void resultant(expression** result, const expression* a, const expression* b, const expression* variable) {
    expression* matrix = new_sylvester_matrix(a, b, variable);
    determinant(result, matrix);
    free_expression(matrix, false);
    any_expression_to_sparse_polynomial(*result, variable);
}
