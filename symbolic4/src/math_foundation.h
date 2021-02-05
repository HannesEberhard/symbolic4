
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

#ifndef math_foundation_h
#define math_foundation_h

typedef struct {
    double x;
    double y;
} cartesian_complex;

typedef struct {
    double modulus;
    double argument;
} polar_complex;

uintmax_t min(uintmax_t a, uintmax_t b);
uintmax_t max(uintmax_t a, uintmax_t b);
uintmax_t gcd(uintmax_t a, uintmax_t b);
uintmax_t lcm(uintmax_t a, uintmax_t b);
uintmax_t* prime_factors(uintmax_t a);
uintmax_t* binomial_coefficients(uint8_t n);
return_status addition(uintmax_t* result, uintmax_t a, uintmax_t b);
return_status multiplication(uintmax_t* result, uintmax_t a, uintmax_t b);
return_status exponentiation(uintmax_t* result, uintmax_t base, uintmax_t exponent);
void root_extraction(uintmax_t* factor, uintmax_t* remainder, uintmax_t base, uintmax_t degree);
polar_complex c2p_complex(cartesian_complex source);
cartesian_complex p2c_complex(polar_complex source);
cartesian_complex complex_add(cartesian_complex a, cartesian_complex b);
cartesian_complex complex_sub(cartesian_complex a, cartesian_complex b);
polar_complex complex_mul(polar_complex a, polar_complex b);
polar_complex complex_div(polar_complex a, polar_complex b);
polar_complex complex_exp(polar_complex a, uint8_t n);
cartesian_complex complex_poly(const double* coefficients, uint8_t degree, cartesian_complex x);

#endif /* math_foundation_h */
