
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

#ifndef foundation_h
#define foundation_h

#include "symbolic4.h"

#define ERROR_CHECK(F) if ((F) == RETS_ERROR) return RETS_ERROR ///< Check if the return status of a function is @c RETS_ERROR. If so, return @c RETS_ERROR.

typedef enum {
    ERRD_NULL,
    ERRD_SYSTEM,
    ERRD_SYNTAX,
    ERRD_MATH,
    ERRD_EXPRESSION,
    ERRD_PARSER,
    ERRD_SIMPLFY,
    ERRD_SOLVE,
    ERRD_DERIVATIVE,
    ERRD_INTEGRAL,
    ERRD_POLY,
    ERRD_MATRIX,
    ERRD_VECTOR
} error_domain;

typedef enum {
    ERRI_NULL,
    ERRI_MEMORY_ALLOCATION,
    ERRI_UNEXPECTED_CHARACTER,
    ERRI_UNEXPECTED_EXPRESSION,
    ERRI_FUNCTION_CALL_SYNTAX,
    ERRI_SYNTAX,
    ERRI_PARENTHESIS_MISMATCH,
    ERRI_MAX_INT_VALUE_EXCEEDED,
    ERRI_ARGUMENTS,
    ERRI_VECTOR_DIMENSIONS,
    ERRI_UNDEFINED_VALUE,
    ERRI_NON_DIFFERENTIABLE
} error_identifier;

typedef enum {
    RETS_NULL,
    RETS_ERROR,
    RETS_SUCCESS,
    RETS_CHANGED,
    RETS_UNCHANGED
} return_status;

typedef struct {
    error_domain domain;
    error_identifier identifier;
    char body[20];
} error;

extern error current_error;
extern void** allocated_pointers;
extern bool smart_alloc_is_recording;

void* smart_alloc(uint8_t length, size_t size);
void* smart_realloc(void* source, uint8_t length, size_t size);
void smart_free(void* pointer);
void smart_free_all(void);
uint8_t set_error(error_domain domain, error_identifier identifier, const char* body);
void set_handle_unrecoverable_error(error_domain domain, error_identifier identifier, const char* body);
double uintmax_max_value(void);
char* string_to_lower(const char* string);
void itoa(char* buffer, uintmax_t source);
void dtoa(char* buffer, uint8_t length, double source);

#endif /* foundation_h */
