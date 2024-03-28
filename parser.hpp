#pragma once

#include <cstdio> // C++ header for standard input/output.
#include <cstdlib> // C++ header for standard library functions.
#include <cstdbool> // C++ header for boolean type.

#include "tokenqueue.hpp" // Assuming "tokenqueue.h" has been made compatible with C++

namespace Parser {

// Initializes the parser. Call this once before you start calling parser_parse().
void parser_init();

// Parses the given input stream using the scanner to obtain tokens and then checks
// the syntax of the input against the BNF rules for the subset of Python being supported.
// Returns nullptr if a syntax error was found (in this case, an error message is output).
// Returns a pointer to a list of tokens -- a Token Queue -- if no syntax errors were detected.
// This queue contains the complete input in token form for analysis and execution.
// NOTE: It is the caller's responsibility to free the resources used by the Token Queue.
TokenQueue* parser_parse(FILE* input);

} // namespace Parser
