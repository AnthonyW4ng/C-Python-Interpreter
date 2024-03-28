#pragma once

#include <cstring> 
#include <cstdlib> 
#include <iostream> 

namespace Utilities {

// Outputs the given error message to stdout and then
// exits the program with an error code of -1. As a result,
// this function never returns.
void panic(char* msg);

// Duplicates the given string and returns a pointer
// to the copy. The caller takes ownership of the copy
// and must eventually free that memory.
char* dupString(const char* s);

// Given 2 strings, makes a copy by concatenating
// them together, and returns the copy. The caller takes
// ownership of the copy and must eventually free that memory.
char* dupStrings(const char* s1, const char* s2);

// Duplicates the given string and returns a pointer
// to the copy; any EOLN characters (\n, \r, etc.)
// are also removed. The caller takes ownership of the
// copy and must eventually free that memory.
char* dupAndStripEOLN(const char* s);

// Case-insensitive comparison of strings s1 and s2.
// Like strcmp, returns 0 if s1 == s2 and returns a
// non-zero value if s1 != s2. Example: icmpStrings("apple", "APPLE") returns 0
int icmpStrings(const char* s1, const char* s2);

} // namespace Utilities
