
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

double n_evaluate_literal(const expression* source);
double n_evaluate_symbol(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_addition(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_subtraction(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_multiplication(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_division(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_exponentiation(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_abs(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_ln(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_log(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_sin(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_cos(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_tan(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_arcsin(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_arccos(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_arctan(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_polynomial(const expression* source, int symbol_count, va_list* symbols);
double n_evaluate_(const expression* source, int symbol_count, va_list* symbols);
uint8_t n_polynomial_to_coefficients(double** coefficients, const expression* source);
double* n_polynomial_make_monic(const double* coefficients, uint8_t degree);
cartesian_complex* n_durand_kerner(const double* coefficients, uint8_t degree);
uint8_t n_poly_real_roots(double** roots, const expression* source, double lower_bound, double upper_bound);
double n_bisection(const expression* source, const char* variable, double lower_bound, double upper_bound);
uint8_t n_linear_search_bisection(double** roots, const expression* source, const char* variable, double lower_bound, double upper_bound);

bool is_numerical(const expression* source, int symbol_count, ...) {
    
    va_list symbols;
    bool result;
    
    va_start(symbols, symbol_count);
    result = isfinite(n_evaluate_(source, symbol_count, &symbols));
    va_end(symbols);
    
    return result;
    
}

double n_evaluate_literal(const expression* source) {
    return (double) source->sign * (double) source->value.numeric.numerator / (double) source->value.numeric.denominator;
}

double n_evaluate_symbol(const expression* source, int symbol_count, va_list* symbols) {
    
    uint8_t i;
    va_list temp_symbols;
    char* symbol;
    double value = NAN;

    if (strcmp(source->value.symbolic, "pi") == 0) {
        return M_PI;
    } else if (strcmp(source->value.symbolic, "e") == 0) {
        return M_E;
    } else if (symbol_count > 0) {
        va_copy(temp_symbols, *symbols);
        for (i = 0; i < symbol_count; i++) {
            symbol = va_arg(temp_symbols, char*);
            value = va_arg(temp_symbols, double);
            if (strcmp(symbol, source->value.symbolic) == 0) {
                va_end(temp_symbols);
                return value;
            }
        }
        va_end(temp_symbols);
        return NAN;
    } else {
        return NAN;
    }
    
}

double n_evaluate_addition(const expression* source, int symbol_count, va_list* symbols) {

    uint8_t i;
    double result = 0;
    
    for (i = 0; i < source->child_count; i++) {
        result += n_evaluate_(source->children[i], symbol_count, symbols);
    }

    return result;

}

double n_evaluate_subtraction(const expression* source, int symbol_count, va_list* symbols) {
    return n_evaluate_(source->children[0], symbol_count, symbols) - n_evaluate_(source->children[1], symbol_count, symbols);
}

double n_evaluate_multiplication(const expression* source, int symbol_count, va_list* symbols) {
    
    uint8_t i;
    double result = 1;
    
    for (i = 0; i < source->child_count; i++) {
        result *= n_evaluate_(source->children[i], symbol_count, symbols);
    }
    
    return result;
    
}

double n_evaluate_division(const expression* source, int symbol_count, va_list* symbols) {
    return n_evaluate_(source->children[0], symbol_count, symbols) / n_evaluate_(source->children[1], symbol_count, symbols);
}

double n_evaluate_exponentiation(const expression* source, int symbol_count, va_list* symbols) {
    return pow(n_evaluate_(source->children[0], symbol_count, symbols), n_evaluate_(source->children[1], symbol_count, symbols));
}

double n_evaluate_abs(const expression* source, int symbol_count, va_list* symbols) {
    return fabs(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_ln(const expression* source, int symbol_count, va_list* symbols) {
    return log(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_log(const expression* source, int symbol_count, va_list* symbols) {
    if (source->child_count == 1) {
        return log10(n_evaluate_(source->children[0], symbol_count, symbols));
    } else {
        return log(n_evaluate_(source->children[0], symbol_count, symbols)) / log(n_evaluate_(source->children[1], symbol_count, symbols));
    }
}

double n_evaluate_sin(const expression* source, int symbol_count, va_list* symbols) {
    return sin(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_cos(const expression* source, int symbol_count, va_list* symbols) {
    return cos(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_tan(const expression* source, int symbol_count, va_list* symbols) {
    return tan(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_arcsin(const expression* source, int symbol_count, va_list* symbols) {
    return asin(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_arccos(const expression* source, int symbol_count, va_list* symbols) {
    return acos(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_arctan(const expression* source, int symbol_count, va_list* symbols) {
    return atan(n_evaluate_(source->children[0], symbol_count, symbols));
}

double n_evaluate_polynomial(const expression* source, int symbol_count, va_list* symbols) {
    
    uint8_t i;
    double base = n_evaluate_(source->children[1], symbol_count, symbols);
    double result = 0;
    
    for (i = 0; i < source->children[0]->child_count; i++) {
        result += n_evaluate_(source->children[0]->children[i], symbol_count, symbols) * pow(base, (double) i);
    }
    
    return result;
    
}

double n_evaluate_(const expression* source, int symbol_count, va_list* symbols) {
    switch (source->identifier) {
        case EXPI_LITERAL: return n_evaluate_literal(source);
        case EXPI_SYMBOL: return n_evaluate_symbol(source, symbol_count, symbols);
        case EXPI_ADDITION: return n_evaluate_addition(source, symbol_count, symbols);
        case EXPI_SUBTRACTION: return n_evaluate_subtraction(source, symbol_count, symbols);
        case EXPI_MULTIPLICATION: return n_evaluate_multiplication(source, symbol_count, symbols);
        case EXPI_DIVISION: return n_evaluate_division(source, symbol_count, symbols);
        case EXPI_EXPONENTIATION: return n_evaluate_exponentiation(source, symbol_count, symbols);
        case EXPI_ABS: return n_evaluate_abs(source, symbol_count, symbols);
        case EXPI_LN: return n_evaluate_ln(source, symbol_count, symbols);
        case EXPI_LOG: return n_evaluate_log(source, symbol_count, symbols);
        case EXPI_SIN: return n_evaluate_sin(source, symbol_count, symbols);
        case EXPI_COS: return n_evaluate_cos(source, symbol_count, symbols);
        case EXPI_TAN: return n_evaluate_tan(source, symbol_count, symbols);
        case EXPI_ARCSIN: return n_evaluate_arcsin(source, symbol_count, symbols);
        case EXPI_ARCCOS: return n_evaluate_arccos(source, symbol_count, symbols);
        case EXPI_ARCTAN: return n_evaluate_arctan(source, symbol_count, symbols);
        case EXPI_POLYNOMIAL: return n_evaluate_polynomial(source, symbol_count, symbols);
        default: return NAN;
    }
}

double n_evaluate(const expression* source, int symbol_count, ...) {
    
    va_list symbols;
    double result;
    
    va_start(symbols, symbol_count);
    result = n_evaluate_(source, symbol_count, &symbols);
    va_end(symbols);
    
    return result;
    
}

uint8_t n_polynomial_to_coefficients(double** coefficients, const expression* source) {
    
    uint8_t i;
    uint8_t degree = 0;
    
    *coefficients = smart_alloc(source->children[0]->child_count, sizeof(double));
    
    for (i = 0; i < source->children[0]->child_count; i++) {
        (*coefficients)[i] = n_evaluate(source->children[0]->children[i], 0);
        if (!are_equal((*coefficients)[i], 0)) {
            degree = i;
        }
    }
    
    return degree;
    
}

double* n_polynomial_make_monic(const double* coefficients, uint8_t degree) {
    
    uint8_t i;
    double* monic_coefficients = smart_alloc(degree + 1, sizeof(double));
    
    for (i = 0; i < degree; i++) monic_coefficients[i] = coefficients[i] / coefficients[degree];
    monic_coefficients[degree] = 1;
    
    return monic_coefficients;
    
}

cartesian_complex* n_durand_kerner(const double* coefficients, uint8_t degree) {
    
    uint8_t i, j, k;
    double* monic_coefficients = n_polynomial_make_monic(coefficients, degree);
    polar_complex base_root = c2p_complex((cartesian_complex) {0.4, 0.9});
    polar_complex product;
    cartesian_complex* roots = smart_alloc(degree, sizeof(cartesian_complex));
    cartesian_complex* temp_roots = smart_alloc(degree, sizeof(cartesian_complex));
    
    for (i = 0; i < degree; i++) {
        roots[i] = p2c_complex(complex_exp(base_root, i));
    }

    for (i = 0; i < 100; i++) {

        for (j = 0; j < degree; j++) {
            product = (polar_complex) {1, 0};
            for (k = 0; k < degree; k++) {
                if (j != k) {
                    product = complex_mul(product, c2p_complex(complex_sub(roots[j], roots[k])));
                }
            }
            temp_roots[j] = complex_sub(roots[j],p2c_complex(complex_div(c2p_complex(complex_poly(monic_coefficients, degree, roots[j])), product)));
        }

        for (j = 0; j < degree; j++) {
            roots[j] = temp_roots[j];
        }

    }

    smart_free(monic_coefficients);
    smart_free(temp_roots);
    
    return roots;
    
}

uint8_t n_poly_real_roots(double** roots, const expression* source, double lower_bound, double upper_bound) {
    
    uint8_t i;
    uint8_t degree;
    uint8_t root_count = 0;
    double* coefficients;
    cartesian_complex* complex_roots;
    
    degree = n_polynomial_to_coefficients(&coefficients, source);
    complex_roots = n_durand_kerner(coefficients, degree);
    *roots = smart_alloc(degree, sizeof(double));
    
    for (i = 0; i < degree; i++) {
        if (fabs(complex_roots[i].y) < sqrt(DBL_EPSILON) &&
            complex_roots[i].x > lower_bound + DBL_EPSILON &&
            complex_roots[i].x < upper_bound - DBL_EPSILON) {
            (*roots)[root_count++] = complex_roots[i].x;
        }
    }
    
    smart_realloc(*roots, (root_count == 0) ? 1 : root_count, sizeof(double));
    smart_free(coefficients);
    smart_free(complex_roots);
    
    return root_count;
    
}

double n_bisection(const expression* source, const char* variable, double lower_bound, double upper_bound) {
    
    double center = (upper_bound - lower_bound) / 2;
    double f_center;
    double f_lower_bound = n_evaluate(source, 1, variable, lower_bound);
    
    if (!isfinite(f_lower_bound) || lower_bound >= upper_bound || f_lower_bound * n_evaluate(source, 1, variable, upper_bound) >= 0) {
        return NAN;
    }
    
    while (upper_bound - lower_bound > sqrt(DBL_EPSILON)) {
        center = (upper_bound + lower_bound) / 2;
        f_center = n_evaluate(source, 1, variable, center);
        if (!isfinite(f_center)) {
            return NAN;
        } else if (fabs(f_center) < sqrt(DBL_EPSILON)) {
            return center;
        } else if (f_center * f_lower_bound < 0) {
            upper_bound = center;
        } else {
            lower_bound = center;
            f_lower_bound = f_center;
        }
    }
    
    return center;
    
}

uint8_t n_linear_search_bisection(double** roots, const expression* source, const char* variable, double lower_bound, double upper_bound) {
    
    double x1;
    double f_x1, f_x2;
    double dx = 1e-4;
    double root;
    uint8_t root_count = 0;
    
    *roots = smart_alloc(1, sizeof(double));
    f_x1 = n_evaluate(source, 1, variable, lower_bound);
    
    for (x1 = lower_bound; x1 + dx <= upper_bound; x1 += dx) {
        f_x2 = n_evaluate(source, 1, variable, x1 + dx);
        if (isfinite(f_x1) && isfinite(f_x2) && f_x1 * f_x2 < 0) {
            if (isfinite(root = n_bisection(source, variable, x1, x1 + dx))) {
                *roots = smart_realloc(*roots, root_count + 1, sizeof(double));
                (*roots)[root_count++] = root;
            }
        }
        f_x1 = f_x2;
    }
    
    return root_count;
    
}

uint8_t n_roots(double** roots, const expression* source, const char* variable, double lower_bound, double upper_bound) {
    if (source->identifier == EXPI_POLYNOMIAL) {
        return n_poly_real_roots(roots, source, lower_bound, upper_bound);
    } else {
        return n_linear_search_bisection(roots, source, variable, lower_bound, upper_bound);
    }
}

double n_derivative(const expression* source, const char* variable, double x) {
    double h = sqrt(DBL_EPSILON);
    return (n_evaluate(source, 1, variable, x + h) - n_evaluate(source, 1, variable, x - h)) / (2 * h);
}

double n_integral(const expression* source, const char* variable, double lower_bound, double upper_bound) {
    
    uint8_t i;
    double integral = 0;
    
    double abscissae[] = {
        0,
        -0.201194093997434,
        0.201194093997434,
        -0.394151347077563,
        0.394151347077563,
        -0.570972172608538,
        0.570972172608538,
        -0.72441773136017,
        0.72441773136017,
        -0.848206583410427,
        0.848206583410427,
        -0.937273392400706,
        0.937273392400706,
        -0.987992518020485,
        0.987992518020485,
    };
    
    double weights[] = {
        0.202578241925561,
        0.198431485327111,
        0.198431485327111,
        0.186161000015562,
        0.186161000015562,
        0.166269205816993,
        0.166269205816993,
        0.139570677926154,
        0.139570677926154,
        0.107159220467171,
        0.107159220467171,
        0.0703660474881081,
        0.0703660474881081,
        0.0307532419961173,
        0.0307532419961173
    };
    
    for (i = 0; i < sizeof(abscissae) / sizeof(double); i++) {
        integral += n_evaluate(source, 1, variable, ((upper_bound - lower_bound) * abscissae[i] + lower_bound + upper_bound) / 2) * weights[i];
    }
    
    return integral * (upper_bound - lower_bound) / 2;
    
}

double n_area(const expression* source, const char* variable, double lower_bound, double upper_bound) {
    
    uint8_t i;
    uint8_t root_count;
    double* roots;
    double area;
    
    root_count = n_roots(&roots, source, variable, lower_bound, upper_bound);
    
    if (root_count == 0) {
        area = fabs(n_integral(source, variable, lower_bound, upper_bound));
    } else if (root_count == 1) {
        area = fabs(n_integral(source, variable, lower_bound, roots[0]));
        area += fabs(n_integral(source, variable, roots[0], upper_bound));
    } else {
        area = fabs(n_integral(source, variable, lower_bound, roots[0]));
        area += fabs(n_integral(source, variable, roots[root_count - 1], upper_bound));
        for (i = 0; i < root_count - 1; i++) {
            area += fabs(n_integral(source, variable, roots[i], roots[i + 1]));
        }
    }
    
    smart_free(roots);
    
    return area;
    
}
