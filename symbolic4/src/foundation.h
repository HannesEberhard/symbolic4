
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

#ifndef foundation_h
#define foundation_h

//#define SMART_MEMORY
#define ERROR_CHECK(F) if ((F) == RETS_ERROR) return RETS_ERROR

#define ERRORS \
X(ERRI_NULL, "%s") \
X(ERRI_IS_NOT_IMPLEMENTED, "%s is not implemented.") \
X(ERRI_MEMORY_ALLOCATION_FAILED, "Dynamic memory allocation failed.") \
X(ERRI_MEMORY_REALLOCATION_FAILED, "Dynamic memory reallocation failed.") \
X(ERRI_MAX_NUMBER_OF_POINTERS_EXCEEDED, "Max number of pointers was exceeded.") \
X(ERRI_CREATE_LITERAL_WITH_DENOMINATOR_ZERO, "Attempting to create literal %u / 0.") \
X(ERRI_LITERAL_WITH_DENOMINATOR_ZERO, "Found literal %u / 0.") \
X(ERRI_INTEGER_OVERFLOW_WHILE_PARSING, "Integer overflow detected while parsing \"%s\".") \
X(ERRI_UNEXPECTED_CHARACTER, "Unexpected character found: '%c'.") \
X(ERRI_EMPTY_INPUT, "Empty input detected.") \
X(ERRI_INVALID_QUERY_START, "Queries must not start with \"%s\".") \
X(ERRI_INVALID_QUERY_ENDING, "Queries must not end with \"%s\".") \
X(ERRI_INVALID_SEQUENCE, "\"%s\" must not be followed by \"%s\".") \
X(ERRI_MISPLACED_COMMA, "Misplaced comma found.") \
X(ERRI_TOO_MANY_RIGHT_PARENTHESIS, "Too many right parenthesis found.") \
X(ERRI_INTEGER_OVERFLOW, "Integer overflow detected.")

typedef enum {
#define X(enum_value, string_value) enum_value,
    ERRORS
#undef X
} error_identifier;

static char* error_messages[] = {
#define X(enum_value, string_value) string_value,
    ERRORS
#undef X
};

extern void** allocated_pointers;
extern char last_error_message[250];

void init(void);
void cleanup(void);
void* smart_alloc(uint8_t count, size_t size);
void* smart_realloc(void* pointer, uint8_t count, size_t size);
void smart_free(void* pointer);
void print_allocated_pointers(void);
bool are_equal(double a, double b);
return_status set_error(error_identifier identifier, bool fatal, ...);

#endif /* foundation_h */
