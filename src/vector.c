
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

uint8_t vector_magnitude(expression** result, expression* source, bool persistent) {
    
    uint8_t i;
    
    *result = new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                             new_expression(EXPT_OPERATION, EXPI_ADDITION, 0),
                             new_literal(1, 1, 2));
    
    for (i = 0; i < source->child_count; i++) {
        append_child((*result)->children[0], new_expression(EXPT_OPERATION, EXPI_EXPONENTATION, 2,
                                                            copy_expression(source->children[i]),
                                                            new_literal(1, 2, 1)));
    }
    
    simplify(*result, true);
    
    if (!persistent) free_expression(source, false);
    
    return RETS_SUCCESS;
    
}

uint8_t vector_normalized(expression** result, expression* source, expression* magnitude, bool persistent) {
    
    uint8_t i;
    expression* source_magnitude;
    expression* factor;
    
    vector_magnitude(&source_magnitude, source, true);
    
    factor = new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                            copy_expression(magnitude),
                            source_magnitude);
    
    *result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; i < source->child_count; i++) {
        append_child(*result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                             copy_expression(factor),
                                             copy_expression(source->children[i])));
    }
    
    simplify(*result, true);
    
    if (!persistent) {
        free_expression(source, false);
        free_expression(magnitude, false);
    }
    
    return RETS_SUCCESS;
    
}

uint8_t vector_angle(expression** result, expression* source_1, expression* source_2, bool persistent) {
    
    expression* source_1_magnitude;
    expression* source_2_magnitude;
    expression* dot_product;
    
    ERROR_CHECK(vector_magnitude(&source_1_magnitude, source_1, true));
    ERROR_CHECK(vector_magnitude(&source_2_magnitude, source_2, true));
    
    ERROR_CHECK(vector_dot_product(&dot_product, source_1, source_2, true));
    
    *result = new_expression(EXPT_FUNCTION, EXPI_ARCCOS, 1,
                             new_expression(EXPT_OPERATION, EXPI_DIVISION, 2,
                                            dot_product,
                                            new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                           source_1_magnitude,
                                                           source_2_magnitude)));
    
    simplify(*result, true);
    
    if (!persistent) {
        free_expression(source_1, false);
        free_expression(source_2, false);
    }
    
    return RETS_SUCCESS;
    
}

uint8_t vector_dot_product(expression** result, expression* source_1, expression* source_2, bool persistent) {
    
    uint8_t i;
    
    *result = new_expression(EXPT_OPERATION, EXPI_ADDITION, 0);
    
    for (i = 0; i < source_1->child_count; i++) {
        append_child(*result, new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                             copy_expression(source_1->children[i]),
                                             copy_expression(source_2->children[i])));
    }
    
    simplify(*result, true);
    
    if (!persistent) {
        free_expression(source_1, false);
        free_expression(source_2, false);
    }
    
    return RETS_SUCCESS;
    
}

uint8_t vector_cross_product(expression** result, expression* source_1, expression* source_2, bool persistent) {
    
    uint8_t i, j, k;
    
    *result = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    for (i = 0; i < source_1->child_count; i++) {
        j = (i + 1) % 3;
        k = (i + 2) % 3;
        append_child(*result, new_expression(EXPT_OPERATION, EXPI_SUBTRACTION, 2,
                                             new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                            copy_expression(source_1->children[j]),
                                                            copy_expression(source_2->children[k])),
                                             new_expression(EXPT_OPERATION, EXPI_MULTIPLICATION, 2,
                                                            copy_expression(source_1->children[k]),
                                                            copy_expression(source_2->children[j]))));
    }
    
    simplify(*result, true);
    
    if (!persistent) {
        free_expression(source_1, false);
        free_expression(source_2, false);
    }
    
    return RETS_SUCCESS;
    
}

uint8_t vector_triple_product(expression** result, expression* source_1, expression* source_2, expression* source_3, bool persistent) {
    
    ERROR_CHECK(vector_cross_product(result, copy_expression(source_1), copy_expression(source_2), false));
    ERROR_CHECK(vector_dot_product(result, copy_expression(*result), copy_expression(source_3), false));
    
    *result = new_expression(EXPT_FUNCTION, EXPI_ABS, 1, *result);
    
    simplify(*result, true);
    
    return RETS_SUCCESS;
    
}
