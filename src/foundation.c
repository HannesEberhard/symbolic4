
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

error current_error;
void** allocated_pointers;
bool smart_alloc_is_recording = true;

/**
 
 @brief Allocates and keeps track of memory
 
 @details
 The pointer to the allocated memory is stored in the
 @c allocated_pointers array.
 
 @pre
 - The @c allocated_pointers array must be initialized with @c NULL,
 or else the loop won't work.
 
 @warning
 - If the memory allocation fails, @c set_handle_unrecoverable_error() is
 called.
 
 @param[in] length The length/count of the elements.
 @param[in] size The size of one element (see @c sizeof()).
 
 @return
 - A void-pointer to the allocated memory.
 
 @see
 - smart_free()
 - smart_free_all()
 
 */
void* smart_alloc(uint8_t length, size_t size) {
    uint16_t i;
    void* pointer = calloc(length, size);
    if (pointer == NULL) set_handle_unrecoverable_error(ERRD_SYSTEM, ERRI_MEMORY_ALLOCATION, "");
    if (smart_alloc_is_recording) {
        for (i = 0; allocated_pointers[i] != NULL && i < ALLOCATED_POINTERS_LENGTH - 1; i++);
        allocated_pointers[i] = pointer;
    }
    return pointer;
}

/**
 
 @brief Resizes a pointer
 
 @details
 The resized pointer to the allocated memory is stored in the
 @c allocated_pointers array.
 
 @param[in] source The pointer to be resized.
 @param[in] length The new length/count of the elements.
 @param[in] size The size of one element (see @c sizeof()).
 
 @see
 - smart_alloc()
 
 */
void* smart_realloc(void* source, uint8_t length, size_t size) {
    uint16_t i;
    void* pointer = realloc(source, length * size);
    if (pointer == NULL) set_handle_unrecoverable_error(ERRD_SYSTEM, ERRI_MEMORY_ALLOCATION, "");
    if (pointer != source && smart_alloc_is_recording) {
        for (i = 0; allocated_pointers[i] != NULL && i < ALLOCATED_POINTERS_LENGTH; i++);
        allocated_pointers[i] = pointer;
    }
    return pointer;
}

/**
 
 @brief Frees a pointer
 
 @details
 This function frees a pointer and sets its corresponding entry in the
 @c allocated_pointers array to @c NULL.
 
 @param[in] pointer The pointer to be freed.
 
 @see
 - smart_alloc()
 - smart_free_all()
 
 */
void smart_free(void* pointer) {
    uint16_t i;
    if (pointer == NULL) return;
    for (i = 0; allocated_pointers[i] != pointer && i < ALLOCATED_POINTERS_LENGTH; i++);
    free(pointer);
    allocated_pointers[i] = NULL;
}

/**
 
 @brief Frees all pointers
 
 @details
 This function frees all pointers allocated with @c smart_alloc() by
 iteratively calling @c smart_free().
 
 @see
 - smart_alloc()
 - smart_free()
 
 */
void smart_free_all(void) {
    uint16_t i;
    for (i = 0; i < ALLOCATED_POINTERS_LENGTH; i++) {
        if (allocated_pointers[i] != NULL) smart_free(allocated_pointers[i]);
    }
}

/**
 
 @brief Sets @c current_error to the arguments provided
 
 @param[in] domain
 @param[in] identifier
 @param[in] body
 
 @return
 - Returns always @c RETS_ERROR
 
 */
uint8_t set_error(error_domain domain, error_identifier identifier, const char* body) {
    current_error.domain = domain;
    current_error.identifier = identifier;
    strcpy(current_error.body, body);
    return RETS_ERROR;
}

/**
 
 @brief Handles severe and unrecoverable errors
 
 @details
 This function serves as the last resort when the program encounters
 exceptionally severe errors, such as a failed memory allocation,
 from which it can't recover itself.
 The
 @code
 while (true);
 @endcode
 loop may be replaced with system-specific error handling.
 
 @warning
 - This function should only be called if everything else has failed.
 
 @param[in] domain The error domain.
 @param[in] identifier The error identifier.
 @param[in] body The error body.
 
 */
void set_handle_unrecoverable_error(error_domain domain, error_identifier identifier, const char* body) {
    current_error.domain = domain;
    current_error.identifier = identifier;
    strcpy(current_error.body, body);
    while (true);
}

double uintmax_max_value(void) {
    return pow(2, sizeof(uintmax_t) * 8) - 1000;
}

/**
 
 @brief Converts a string to lowercase
 
 @details
 This function copies the source string and converts that copy
 character-wise to lowercase. The resulting string must be freed.
 
 @param[in] string The string to convert.
 
 @return
 - The new string in lowercase letters.
 
 */
char* string_to_lower(const char* string) {
    uint8_t i;
    char* result = smart_alloc(strlen(string) + 1, sizeof(char));
    strcpy(result, string);
    for (i = 0; result[i] != '\0'; i++) {
        result[i] = tolower(result[i]);
    }
    return result;
}

void itoa(char* buffer, uintmax_t source) {
    
    uintmax_t i = 1;
    uint8_t buffer_position = 0;
    
    while (source >= 10 * i) {
        i *= 10;
    }
    
    do {
        buffer[buffer_position++] = (((source - source % i) / i) % 10) + '0';
    } while (i /= 10);
    
    buffer[buffer_position] = '\0';
    
}

void dtoa(char* buffer, uint8_t length, double source) {
    
    uint8_t buffer_position = 0;
    
    if (source < 0) {
        buffer[buffer_position++] = '-';
        source *= -1;
    }
    
    itoa(buffer + buffer_position, source);
    buffer_position = strlen(buffer);
    
    buffer[buffer_position++] = '.';
    
    for (buffer_position = strlen(buffer); buffer_position < length - 1 && source != 0; buffer_position++) {
        source = (source - (uint8_t) source) * 10;
        buffer[buffer_position] = (uint8_t) source + '0';
    }
    
    buffer[length - 1] = '\0';
    
}
