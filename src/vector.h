
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

#ifndef vector_h
#define vector_h

#include "symbolic4.h"

uint8_t vector_magnitude(expression** result, expression* source, bool persistent);
uint8_t vector_normalized(expression** result, expression* source, expression* magnitude, bool persistent);
uint8_t vector_angle(expression** result, expression* source_1, expression* source_2, bool persistent);
uint8_t vector_dot_product(expression** result, expression* source_1, expression* source_2, bool persistent);
uint8_t vector_cross_product(expression** result, expression* source_1, expression* source_2, bool persistent);
uint8_t vector_triple_product(expression** result, expression* source_1, expression* source_2, expression* source_3, bool persistent);

#endif /* vector_h */
