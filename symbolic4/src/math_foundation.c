
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

uintmax_t min(uintmax_t a, uintmax_t b) {
    return (a < b) ? a : b;
}

uintmax_t max(uintmax_t a, uintmax_t b) {
    return (a > b) ? a : b;
}

uintmax_t gcd(uintmax_t a, uintmax_t b) {
    
    uintmax_t t;
    
    while (b != 0) {
        t = b;
        b = a % b;
        a = t;
    }
    
    return a;
    
}

uintmax_t lcm(uintmax_t a, uintmax_t b) {
    return (a * b) / gcd(a, b);
}

uintmax_t* prime_factors(uintmax_t a) {
    
    uintmax_t factor = 2;
    uint8_t exponent = 0;
    uintmax_t* result = smart_alloc(1, sizeof(uintmax_t));
    result[0] = 0;
    
    while (a % factor == 0) {
        exponent++;
        a /= factor;
    }
    
    if (exponent != 0) {
        result = smart_realloc(result, 2 * result[0] + 3, sizeof(uintmax_t));
        result[2 * result[0] + 1] = factor;
        result[2 * result[0] + 2] = exponent;
        result[0]++;
        exponent = 0;
    }
    
    for (factor = 3; factor * factor <= a; factor += 2) {
        
        while (a % factor == 0) {
            exponent++;
            a /= factor;
        }
        
        if (exponent != 0) {
            result = smart_realloc(result, 2 * result[0] + 3, sizeof(uintmax_t));
            result[2 * result[0] + 1] = factor;
            result[2 * result[0] + 2] = exponent;
            result[0]++;
            exponent = 0;
        }
        
    }
    
    if (a > 2) {
        result = smart_realloc(result, 2 * result[0] + 3, sizeof(uintmax_t));
        result[2 * result[0] + 1] = a;
        result[2 * result[0] + 2] = 1;
        result[0]++;
    }
    
    return result;
    
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

return_status addition(uintmax_t* result, uintmax_t a, uintmax_t b) {
    if (__builtin_add_overflow(a, b, result)) {
        return set_error(ERRI_INTEGER_OVERFLOW, false);
    } else {
        return RETS_SUCCESS;
    }
}

return_status multiplication(uintmax_t* result, uintmax_t a, uintmax_t b) {
    if (__builtin_mul_overflow(a, b, result)) {
        return set_error(ERRI_INTEGER_OVERFLOW, false);
    } else {
        return RETS_SUCCESS;
    }
}

return_status exponentiation(uintmax_t* result, uintmax_t base, uintmax_t exponent) {
    
    *result = 1;
    
    if (exponent == 0) {
        return RETS_SUCCESS;
    }
    
    while (exponent > 1) {
        if (exponent % 2 == 0) {
            if (__builtin_mul_overflow(base, base, &base)) return set_error(ERRI_INTEGER_OVERFLOW, false);
        } else {
            if (__builtin_mul_overflow(base, *result, result)) return set_error(ERRI_INTEGER_OVERFLOW, false);
            if (__builtin_mul_overflow(base, base, &base)) return set_error(ERRI_INTEGER_OVERFLOW, false);
            exponent--;
        }
        exponent /= 2;
    }
    
    if (__builtin_mul_overflow(base, *result, result)) {
        return set_error(ERRI_INTEGER_OVERFLOW, false);
    } else {
        return RETS_SUCCESS;
    }
    
}

void root_extraction(uintmax_t* factor, uintmax_t* remainder, uintmax_t base, uintmax_t degree) {
    
    uint8_t i;
    uintmax_t temp;
    uintmax_t factor_base;
    uintmax_t factor_exponent;
    uintmax_t* factors = prime_factors(base);
    
    *factor = 1;
    *remainder = 1;
    
    for (i = 0; i < factors[0]; i++) {
        factor_base = factors[2 * i + 1];
        factor_exponent = factors[2 * i + 2];
        exponentiation(&temp, factor_base, factor_exponent / degree);
        multiplication(factor, *factor, temp);
        exponentiation(&temp, factor_base, factor_exponent - (factor_exponent / degree) * degree);
        multiplication(remainder, *remainder, temp);
    }
    
    smart_free(factors);
    
}

polar_complex c2p_complex(cartesian_complex source) {
    double modulus = sqrt(pow(source.x, 2) + pow(source.y, 2));
    double argument = atan(source.y / source.x);
    if (source.x == 0 && source.y == 0) argument = 0;
    if (source.x < 0) argument += M_PI;
    return (polar_complex) {modulus, argument};
}

cartesian_complex p2c_complex(polar_complex source) {
    double x = source.modulus * cos(source.argument);
    double y = source.modulus * sin(source.argument);
    return (cartesian_complex) {x, y};
}

cartesian_complex complex_add(cartesian_complex a, cartesian_complex b) {
    double x = a.x + b.x;
    double y = a.y + b.y;
    return (cartesian_complex) {x, y};
}

cartesian_complex complex_sub(cartesian_complex a, cartesian_complex b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    return (cartesian_complex) {x, y};
}

polar_complex complex_mul(polar_complex a, polar_complex b) {
    double modulus = a.modulus * b.modulus;
    double argument = a.argument + b.argument;
    return (polar_complex) {modulus, argument};
}

polar_complex complex_div(polar_complex a, polar_complex b) {
    double modulus = a.modulus / b.modulus;
    double argument = a.argument - b.argument;
    return (polar_complex) {modulus, argument};
}

polar_complex complex_exp(polar_complex a, uint8_t n) {
    double modulus = pow(a.modulus, (double) n);
    double argument = a.argument * (double) n;
    return (polar_complex) {modulus, argument};
}

cartesian_complex complex_poly(const double* coefficients, uint8_t degree, cartesian_complex x) {
    
    uint8_t i;
    polar_complex x_polar = c2p_complex(x);
    cartesian_complex result = (cartesian_complex) {0, 0};
    
    for (i = 0; i < degree + 1; i++) {
        result = complex_add(result, p2c_complex(complex_mul((polar_complex) {coefficients[i], 0}, complex_exp(x_polar, i))));
    }
    
    return result;
    
}
