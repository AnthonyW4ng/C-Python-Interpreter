#pragma once

#include <cstdio> // for FILE
#include "token.hpp" 

namespace Scanner {

// Initializes line number, column number, and value before
// the start of processing the next input stream.
// Parameters are pointers to allow for modification.
void scanner_init(int* lineNumber, int* colNumber, char* value);

// Returns the next token in the given input stream, advancing the line
// number and column number as appropriate. The token's string-based
// value is returned via the "value" parameter.
// Returns a Token structure, assuming Token is compatible with C++.
Token scanner_nextToken(FILE* input, int* lineNumber, int* colNumber, char* value);

} // namespace Scanner
