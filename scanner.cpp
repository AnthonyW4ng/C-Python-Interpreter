#include <cassert>  
#include <cctype>  
#include <cstdbool> 
#include <cstdio>   
#include <cstring>  
#include <string>   
#include <vector>   

#include "scanner.hpp" 
#include "util.hpp"

namespace nuPython {

// Collects the rest of an identifier into value while advancing the column number.
static void collect_identifier(FILE *input, int c, int *colNumber, std::string& value) {
    assert(isalpha(c) || c == '_'); // c should be start of identifier

    value.clear(); // Ensure the string is empty before collecting the identifier

    while (isalnum(c) || c == '_') { // letter, digit, or underscore
        value += static_cast<char>(c); // store char

        (*colNumber)++; // advance col # past char

        c = fgetc(input); // get next char
    }

    // Put the last char back for processing later:
    ungetc(c, input);
}

// Determines whether the input value is a nuPython keyword or identifier.
static int id_or_keyword(const std::string& value) {
    assert(!value.empty()); // valid value?

    // Keywords in the same order as the tokens.h enum.
    std::vector<std::string> keywords = {"and",   "break", "continue", "def",    "elif", "else",
                                         "False", "for",   "if",       "in",     "is",   "None",
                                         "not",   "or",    "pass",     "return", "True", "while"};

    auto it = std::find(keywords.begin(), keywords.end(), value);

    if (it == keywords.end())
        return nuPy_IDENTIFIER;
    else
        return nuPy_KEYW_AND + std::distance(keywords.begin(), it);
}

} // namespace nuPython

static int collect_numeric_literal(FILE* input, int c, int* colNumber,
                                   std::string& value, int prefix) {
    assert(isdigit(c)); // c should be start of a numeric literal
    assert(prefix >= 0 && prefix <= 2);

    if (prefix == 1) { // '+' literal
        value += '+';
    } else if (prefix == 2) { // '-' literal
        value += '-';
    }

    // Start collecting the integer part:
    while (isdigit(c)) {
        value += static_cast<char>(c); // store char

        (*colNumber)++; // advance col # past char

        c = fgetc(input); // get next char
    }

    // We have the integer part, what stopped the loop?
    if (c != '.') { // Not a real literal, so return this int:
        ungetc(c, input); // Put the char back for processing next
        return nuPy_INT_LITERAL;
    }

    // Asserting for clarity, though not strictly necessary
    assert(c == '.'); // we have a real literal

    // Collecting the decimal point and digits:
    value += '.';
    (*colNumber)++; // advance col # past char

    c = fgetc(input); // get next char

    while (isdigit(c)) {
        value += static_cast<char>(c); // store char
        (*colNumber)++; // advance col # past char
        c = fgetc(input); // get next char
    }

    // Put the last char back for processing later
    ungetc(c, input);

    return nuPy_REAL_LITERAL;
}


static void collect_string_literal(FILE* input, int c, int* colNumber,
                                   std::string& value, int startLine, int startCol) {
    assert(c == '"' || c == '\''); // c should be start of string literal

    int startChar = c;
    (*colNumber)++; // advance col # past start char
    c = fgetc(input); // get next char

    value.clear(); // Ensure the string is empty

    while (c != startChar && c != '\n' && c != EOF) {
        value += static_cast<char>(c); // store char

        (*colNumber)++; // advance col # past char
        c = fgetc(input); // get next char
    }

    // Warn the user if the string literal is not terminated properly
    if (c == '\n' || c == EOF) {
        std::cout << "**WARNING: string literal @ (" << startLine << ", " << startCol << ") not terminated properly\n";
        ungetc(c, input); // put char back
    } else {
        // Properly terminated string
        (*colNumber)++; // advance col # past closing char
    }
}

void scanner_init(int& lineNumber, int& colNumber, std::string& value) {
    // Using references to directly modify the arguments
    lineNumber = 1;
    colNumber = 1;
    value.clear(); // Initialize the string to empty
}

class Token {
public:
    TokenID id;
    int line;
    int col;

    Token() : id(nuPy_EOS), line(0), col(0) {}
};

class Scanner {
public:
    static Token nextToken(std::ifstream& input, int& lineNumber, int& colNumber, std::string& value) {
        if (!input)
            throw std::invalid_argument("Input stream is NULL (nextToken)");
        if (value.empty())
            throw std::invalid_argument("Value string is NULL (nextToken)");

        Token T;

        while (true) {
            int c = input.get();

            if (c == EOF) {
                T.id = nuPy_EOS;
                T.line = lineNumber;
                T.col = colNumber;

                value = "$";
                return T;
            } else if (c == '$') {
                T.id = nuPy_EOS;
                T.line = lineNumber;
                T.col = colNumber;

                colNumber++;

                value = "$";
                return T;
            } else if (c == '\n') {
                lineNumber++;
                colNumber = 1;
                continue;
            } else if (isspace(c)) {
                colNumber++;
                continue;
            } else if (c == '(') {
                T.id = nuPy_LEFT_PAREN;
                T.line = lineNumber;
                T.col = colNumber;

                colNumber++;

                value = static_cast<char>(c);
                return T;
            } else if (c == ')') {
                T.id = nuPy_RIGHT_PAREN;
                T.line = lineNumber;
                T.col = colNumber;

                colNumber++;

                value = static_cast<char>(c);
                return T;
            } else if (isalpha(c) || c == '_') {
                T.id = nuPy_IDENTIFIER;
                T.line = lineNumber;
                T.col = colNumber;

                collectIdentifier(input, static_cast<char>(c), colNumber, value);
                
                T.id = idOrKeyword(value);
                
                return T;
            } else if (c == '[') {
                T.id = nuPy_LEFT_BRACKET;
                T.line = lineNumber;
                T.col = colNumber;

                colNumber++;

                value = static_cast<char>(c);
                return T;
            } else if (c == ']') {
            T.id = nuPy_RIGHT_BRACKET;
            T.line = lineNumber;
            T.col = colNumber;

            colNumber++;

            value = "]";
            return T;
        } else if (c == '{') {
            T.id = nuPy_LEFT_BRACE;
            T.line = lineNumber;
            T.col = colNumber;

            colNumber++;

            value = "{";
            return T;
    } else if (c == '}') {
        T.id = nuPy_RIGHT_BRACE;
        T.line = lineNumber;
        T.col = colNumber;

        colNumber++;

        value = "}";
        return T;
    } else if (c == '+') {
        T.id = nuPy_PLUS;
        T.line = lineNumber;
        T.col = colNumber;

        colNumber++;

        value = "+";
        c = input.peek(); // Use peek to look at the next char without consuming it

        if (isdigit(c)) {
            T.id = collectNumericLiteral(input, c, colNumber, value, true /* isPositive */);
            return T;
        }

        // If not a digit, no need to unget. We didn't advance the stream.
        return T;
    } else if (c == '-') {
        T.id = nuPy_MINUS;
        T.line = lineNumber;
        T.col = colNumber;

        colNumber++;

        value = "-";
        c = input.peek(); // Use peek to look at the next char without consuming it

        if (isdigit(c)) {
            T.id = collectNumericLiteral(input, c, colNumber, value, false /* isPositive */);
            return T;
        }
        else if (c == '=') {
    T.id = nuPy_EQUAL;
    T.line = lineNumber;
    T.col = colNumber;

    colNumber++;
    value = "=";

    c = input.get();
    if (c == '=') {
        T.id = nuPy_EQUALEQUAL;
        colNumber++;
        value += "=";
    } else {
        // If not '=', put it back to the stream to be processed next
        input.putback(c);
    }

    return T;
} else if (c == '!') {
    T.id = nuPy_UNKNOWN; // Initial assumption, may be overridden below
    T.line = lineNumber;
    T.col = colNumber;

    colNumber++;
    value = "!";

    c = input.get();
    if (c == '=') {
        T.id = nuPy_NOTEQUAL;
        colNumber++;
        value += "=";
    } else {
        // If not '=', put it back to the stream to be processed next
        input.putback(c);
    }

    return T;
} else if (c == '<') {
    T.id = nuPy_LT;
    T.line = lineNumber;
    T.col = colNumber;

    colNumber++;
    value = "<";

    c = input.get();
    if (c == '=') {
        T.id = nuPy_LE;
        colNumber++;
        value += "=";
    } else {
        // If not '=', put it back to the stream to be processed next
        input.putback(c);
    }

    return T;
    }

    if (c == '=') {
    T.id = nuPy_LTE;
    colNumber++;
    value += "=";
    return T;
    } else {
    input.putback(c);
    return T;
    }
    } else if (c == '>') {
    T.id = nuPy_GT;
    T.line = lineNumber;
    T.col = colNumber;

    colNumber++;
    value = ">";

    c = input.get();
    if (c == '=') {
        T.id = nuPy_GTE;
        colNumber++;
        value += "=";
    } else {
        input.putback(c);
    }
    return T;
    } else if (c == '#') {
    // Discard the rest of the line for a line comment
    while ((c = input.get()) != '\n' && c != EOF) {
        colNumber++;
    }
    if (c != EOF) {
        input.putback(c); // Ready to process '\n' or EOF in the next iteration
    }
    continue;
    } else if (isdigit(c)) {
    T.id = nuPy_INT_LITERAL;
    T.line = lineNumber;
    T.col = colNumber;

    T.id = collectNumericLiteral(input, c, colNumber, value, false /* isPositive */, true /* isDigitFirst */);
    return T;
    } else if (c == '"' || c == '\'') {
    T.id = nuPy_STR_LITERAL;
    T.line = lineNumber;
    T.col = colNumber;

    collectStringLiteral(input, c, colNumber, value, lineNumber, colNumber);
    return T;
    } else {
    // Handle unknown token
    T.id = nuPy_UNKNOWN;
    T.line = lineNumber;
    T.col = colNumber;

    colNumber++;
    value = static_cast<char>(c);
    return T;
            }   
        } 
        }    
    }
