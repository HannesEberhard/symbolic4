
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

#include <string>
#include <iostream>
#include <vector>
#include <map>

extern "C" {
#include "includes.h"
}

using namespace std;

vector<string> generate_mapping(void) {
    
    int count = 0;
    vector<string> mapping(256);
    
    map<string, int> frequencies = {
        {"+", 20},
        {"-", 10},
        {"*", 20},
        {"/", 10},
        {"^", 10},
        {"x", 20},
        {"y", 5},
        {"z", 5},
        {"a", 5},
        {"b", 5},
        {"c", 5},
        {"0", 10},
        {"1", 10},
        {"2", 10},
        {"5", 10},
        {"10", 10},
        {"25", 5},
        {"0.5", 8},
        {"1.5", 8},
        {"-1", 5},
        {"-2", 5},
        {"-5", 5},
        {")", 15},
    };
    
    for (int i = EXPI_ABS; i <= EXPI_PARSE; i++) {
        mapping[count++] = (string) get_expression_string((expression_identifier) i) + "(";
    }
    
    for (auto const& [key, val] : frequencies) {
        fill(mapping.begin() + count, mapping.begin() + count + val, key + " ");
        count += val;
    }

    return mapping;
    
}

string generate_query(const uint8_t* data, size_t size) {
    
    string query;
    static vector<string> mapping = generate_mapping();
    
    for (int i = 0; i < size; i++) {
        query += mapping[data[i]];
    }
    
    return query;
    
}

extern "C" {

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    
    string query = generate_query(data, size);
    char buffer[result_size];
    
    memset(buffer, 0, result_size);
    
//    cout << query << endl;

    if (symbolic4(buffer, query.c_str()) == RETS_SUCCESS) {
//        printf("%s\n", buffer);
    } else {
//        printf("%s\n", buffer);
    }
    
    return 0;
    
}

}
