
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

TEST(Keywords, AreUnambiguous) {
    
    char* keyword_occurrence;
    bool ambiguity;
    
    for (int i = 0; keyword_strings[i + 1]; i++) {
        for (int j = i + 1; keyword_strings[j]; j++) {
            ambiguity = (keyword_occurrence = strcasestr(keyword_strings[j], keyword_strings[i])) && strlen(keyword_occurrence) == strlen(keyword_strings[i]);
            EXPECT_FALSE(ambiguity) << keyword_strings[i] << " and " << keyword_strings[j] << " are ambiguous.";
        }
    }
    
}

//TEST(ExpressionFactory, NewLiteralWithZeroDenominatorExits) {
//    EXPECT_EXIT(new_literal(1, 1, 0), testing::ExitedWithCode(EXIT_FAILURE), NULL);
//}
