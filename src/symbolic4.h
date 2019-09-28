
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
 
#ifndef symbolic4_h
#define symbolic4_h

#define VERSION "1.0.0"
#define ALLOCATED_POINTERS_LENGTH 2000
//#define DEBUG_EXPRESSION_CHILDREN

#ifdef _WIN32
#elif defined(__APPLE__)
#elif defined(__linux__)
#define uint8_t u_int8_t
#define uint16_t u_int16_t
#define uint32_t u_int32_t
#define uint64_t u_int32_t
#define uintmax_t u_int32_t
#elif defined(__unix__)
#elif defined(__DOXYGEN__)
#elif defined(_EZ80)
#define uint64_t uint32_t
#define uintmax_t uint32_t
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "foundation.h"
#include "expression.h"
#include "polynomial.h"
#include "math_foundation.h"
#include "parser.h"
#include "simplify.h"
#include "solve.h"
#include "derivative.h"
#include "integral.h"
#include "matrix.h"
#include "vector.h"

extern bool use_abbrevations; ///< Determines if abbrevations should be used in the result string (such as "Deriv" instead of "Derivative")
extern bool use_spaces; ///< Determines if spaces should be used in the result string (such as "x + y * z" instead of "x+y*z")
extern char* default_priorities;

uint8_t symbolic4(char* buffer, const char* query);
uint8_t process(expression* source, bool recursive);

#endif /* symbolic4_h */
