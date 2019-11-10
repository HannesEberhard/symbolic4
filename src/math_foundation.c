
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

uintmax_t min(uintmax_t a, uintmax_t b) {
    return (a < b) ? a : b;
}

uintmax_t max(uintmax_t a, uintmax_t b) {
    return (a > b) ? a : b;
}

/**
 
 @brief Computes the GCD of two integers
 
 @details
 This function computes the greatest common divisor of two integers
 using the iterative Euclidean algorithm.
 
 @param[in] a The first integer.
 @param[in] b The second integer.
 
 @return
 - The greatest common divisor.
 
 */
uintmax_t euclidean_gcd(uintmax_t a, uintmax_t b) {
    
    uintmax_t t;
    
    while (b != 0) {
        t = b;
        b = a % b;
        a = t;
    }
    
    return a;
    
}

expression* prime_factors(uintmax_t source) {
    
    uintmax_t factor = 2;
    expression* factors = new_expression(EXPT_STRUCTURE, EXPI_LIST, 0);
    
    if (source % factor == 0) {
        append_child(factors, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                             new_literal(1, factor, 1),
                                             new_literal(1, 0, 1)));
    }
    
    while (source % factor == 0) {
        factors->children[factors->child_count - 1]->children[1]->value.numeric.numerator++;
        source /= factor;
    }
    
    for (factor = 3; factor <= sqrt(source); factor += 2) {
        
        if (source % factor == 0) {
            append_child(factors, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                                 new_literal(1, factor, 1),
                                                 new_literal(1, 0, 1)));
        }
        
        while (source % factor == 0) {
            factors->children[factors->child_count - 1]->children[1]->value.numeric.numerator++;
            source /= factor;
        }
        
    }
    
    if (source > 2) {
        append_child(factors, new_expression(EXPT_STRUCTURE, EXPI_LIST, 2,
                                             new_literal(1, source, 1),
                                             new_literal(1, 1, 1)));
    }
    
    return factors;
    
}

uintmax_t* binomial_coefficients(uint8_t n) {
    
    uint8_t i, j;
    uintmax_t* coefficients = smart_alloc(n + 1, sizeof(uintmax_t));
    memset(coefficients, 0, (n + 1) * sizeof(uintmax_t));
    
    coefficients[0] = 1;
    
    for (i = 1; i <= n; i++) {
        for (j = min(i, n); j > 0; j--) {
            coefficients[j] = coefficients[j] + coefficients[j - 1];
        }
    }
    
    return coefficients;
    
}

uint8_t addition(uintmax_t* result, uintmax_t a, uintmax_t b) {
    if (((double) a + (double) b) < uintmax_max_value()) {
        *result = a + b;
        return RETS_SUCCESS;
    } else {
        return RETS_ERROR;
    }
}

uint8_t multiplication(uintmax_t* result, uintmax_t a, uintmax_t b) {
    if (((double) a * (double) b) < uintmax_max_value()) {
        *result = a * b;
        return RETS_SUCCESS;
    } else {
        return RETS_ERROR;
    }
}

uint8_t int_power(uintmax_t* result, uintmax_t base, uintmax_t exponent) {
    *result = 1;
    if (pow(base, exponent) < uintmax_max_value()) {
        for ( ; exponent > 0; exponent--) *result *= base;
        return RETS_SUCCESS;
    } else {
        return RETS_ERROR;
    }
}

void int_root(uintmax_t* factor, uintmax_t* remainder, uintmax_t base, uintmax_t degree) {
    
    uint8_t i;
    uintmax_t temp_result;
    expression* factors = prime_factors(base);
    
    for (i = 0; i < factors->child_count; i++) {
        while (factors->children[i]->children[1]->value.numeric.numerator >= degree) {
            factors->children[i]->children[1]->value.numeric.numerator -= degree;
            *factor *= factors->children[i]->children[0]->value.numeric.numerator;
            int_power(&temp_result, factors->children[i]->children[0]->value.numeric.numerator, degree);
            *remainder /= temp_result;
        }
    }
    
    free_expression(factors, false);
    
}
