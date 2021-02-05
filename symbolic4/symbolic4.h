
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
 
#ifndef symbolic4_h
#define symbolic4_h

#define VERSION "2.0.0"

#ifdef _WIN32
#elif defined(__APPLE__)
#define va_copy(d,s) __builtin_va_copy(d,s)
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
#include <float.h>

typedef enum {
    RETS_NULL,
    RETS_ERROR,
    RETS_SUCCESS,
    RETS_CHANGED,
    RETS_UNCHANGED
} return_status;

extern bool use_abbreviations;
extern bool use_spaces;
extern bool force_fractions;
extern char* default_priorities;
extern uint16_t allocated_pointers_size;
extern uint16_t query_size;
extern uint16_t result_size;

uint8_t symbolic4(char* buffer, const char* query);

#endif /* symbolic4_h */
