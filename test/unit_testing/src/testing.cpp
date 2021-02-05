
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

#include "testing.hpp"

void ExpectEquivalence(expression* a, expression* b) {
    
    char a_representation[result_size];
    char b_representation[result_size];
    bool result = expressions_are_identical(a, b, true);
    
    memset(a_representation, 0, result_size);
    memset(b_representation, 0, result_size);
    
    serialize(a_representation, a, SERF_INFIX);
    serialize(b_representation, b, SERF_INFIX);
    
    free_expressions(2, a, b);
    
    EXPECT_TRUE(result) << (std::string) a_representation << " and " << (std::string) b_representation << " are not equivalent.";
    
}

void ExpectEquivalenceAfterProcessing(expression* target, expression* source) {
    
    if (simplify(source, true) == RETS_ERROR) {
        FAIL() << "Error while processing.";
    }
    
    ExpectEquivalence(target, source);
    
}

void ExpectEquivalenceAfterProcessing(const char* target, const char* source) {
    
    expression* target_expression;
    expression* source_expression;
    
    if (parse(&target_expression, target) == RETS_ERROR) {
        FAIL() << "Error while parsing " << target;
    }
    
    if (parse(&source_expression, source) == RETS_ERROR) {
        FAIL() << "Error while parsing " << source;
    }
    
    ExpectEquivalenceAfterProcessing(target_expression, source_expression);
    
}
