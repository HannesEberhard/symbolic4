
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

void** allocated_pointers;
char last_error_message[250];

void init(void) {
    
    memset(last_error_message, 0, 250);
    
#ifdef SMART_MEMORY
    allocated_pointers = calloc(allocated_pointers_size, sizeof(void*));
    memset(allocated_pointers, 0, allocated_pointers_size);
#endif
    
}

void cleanup(void) {
#ifdef SMART_MEMORY
    uint16_t i;
    
    for (i = 0; i < allocated_pointers_size; i++) {
        if (allocated_pointers[i] != NULL) {
            smart_free(allocated_pointers[i]);
        }
    }
    
    free(allocated_pointers);
#endif
}

void* smart_alloc(uint8_t count, size_t size) {
    
    uint16_t i;
    void* pointer = calloc(count, size);
    
//    if (pointer == NULL) {
//        set_error(ERRI_MEMORY_ALLOCATION_FAILED, true);
//    }
    
#ifdef SMART_MEMORY
    
    for (i = 0; allocated_pointers[i] != NULL && i < allocated_pointers_size; i++);
    
    if (i == allocated_pointers_size - 1) {
        set_error(ERRI_MAX_NUMBER_OF_POINTERS_EXCEEDED, true);
    }
    
    allocated_pointers[i] = pointer;
    
#endif
    
    return pointer;
    
}

void* smart_realloc(void* pointer, uint8_t count, size_t size) {
    
    uint16_t i;
    void* new_pointer = realloc(pointer, count * size);
    
//    if (new_pointer == NULL) {
//        set_error(ERRI_MEMORY_REALLOCATION_FAILED, true);
//    }
    
#ifdef SMART_MEMORY
    if (new_pointer != pointer) {
        for (i = 0; allocated_pointers[i] != pointer && i < allocated_pointers_size; i++);
        if (i == allocated_pointers_size - 1) {
            for (i = 0; allocated_pointers[i] != NULL && i < allocated_pointers_size; i++);
            if (i == allocated_pointers_size - 1) {
                set_error(ERRI_MAX_NUMBER_OF_POINTERS_EXCEEDED, true);
            }
        }
        allocated_pointers[i] = new_pointer;
    }
#endif
    
    return new_pointer;
    
}

void smart_free(void* pointer) {
    
    uint16_t i;
    if (pointer == NULL) return;
    
#ifdef SMART_MEMORY
    for (i = 0; allocated_pointers[i] != pointer && i < allocated_pointers_size; i++);
    if (i != allocated_pointers_size - 1) {
        allocated_pointers[i] = NULL;
    }
#endif
    
    free(pointer);
    
}

void print_allocated_pointers(void) {
//#ifdef DEBUG
#ifdef SMART_MEMORY
    uint16_t i;
    for (i = 0; i < allocated_pointers_size; i++) {
        if (allocated_pointers[i] == NULL) continue;
        printf("%i @ %p\n", i, allocated_pointers[i]);
    }
#endif
//#endif
}

bool are_equal(double a, double b) {
    return fabs(a - b) < DBL_EPSILON;
}

return_status set_error(error_identifier identifier, bool fatal, ...) {
    
    va_list args;
    
    va_start(args, fatal);
    vsprintf(last_error_message, error_messages[identifier], args);
    va_end(args);
    
    if (fatal) {
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    return RETS_ERROR;
    
}
