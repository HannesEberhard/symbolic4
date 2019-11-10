
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

#ifndef polynomial_h
#define polynomial_h

#include "symbolic4.h"

void any_expression_to_expression(expression* source);
void any_expression_to_expression_recursive(expression* source);
return_status any_expression_to_sparse_polynomial(expression* source, const expression* variable);
return_status any_expression_to_dense_polynomial(expression* source, const expression* variable);
return_status validate_sparse_polynomial(expression* source, bool allow_decimal_exponents, bool allow_negative_exponents, bool allow_arbitrary_base);
void sort_sparse_polynomial(expression* source);
return_status polysolve(expression* source, expression* variable);
uint8_t poly_div(expression** quotient, expression** remainder, const expression* a, const expression* b, int8_t degree);
bool poly_is_square_free(expression* source);
void make_monic(expression* source);
uint8_t poly_gcd(expression** gcd, const expression* a, const expression* b);
void poly_log_gcd(expression** gcd, const expression* source);
uint8_t factor_square_free(expression** factors, const expression* source);

#endif /* polynomial_h */
