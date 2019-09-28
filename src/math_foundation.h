
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

#ifndef math_foundation_h
#define math_foundation_h

#include "symbolic4.h"

uintmax_t min(uintmax_t a, uintmax_t b);
uintmax_t max(uintmax_t a, uintmax_t b);
uintmax_t euclidean_gcd(uintmax_t a, uintmax_t b);
expression* prime_factors(uintmax_t source);
uintmax_t* binomial_coefficients(uint8_t n);
uint8_t addition(uintmax_t* result, uintmax_t a, uintmax_t b);
uint8_t multiplication(uintmax_t* result, uintmax_t a, uintmax_t b);
uint8_t int_power(uintmax_t* result, uintmax_t base, uintmax_t exponent);
void int_root(uintmax_t* factor, uintmax_t* remainder, uintmax_t base, uintmax_t degree);

#endif /* math_foundation_h */
