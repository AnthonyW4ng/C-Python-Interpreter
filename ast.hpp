#pragma once

#include <string>
#include <memory>
#include "tokenqueue.hpp" 

class AST_EXPR; // Forward declaration

class AST_STMT_ASSIGNMENT {
public:
    std::string variable_name;
    std::unique_ptr<AST_EXPR> expr;
};

class AST_STMT_FUNCTION_CALL {
public:
    std::string function_name;
    std::unique_ptr<AST_EXPR> param;
};

class AST_STMT_WHILE_LOOP {
public:
    std::unique_ptr<AST_EXPR> condition;
    std::unique_ptr<AST_STMT> body;
};

class AST_STMT_IF_THEN_ELSE {
public:
    std::unique_ptr<AST_EXPR> condition;
    std::unique_ptr<AST_STMT> body;
    std::unique_ptr<AST_STMT> elif_else_chain;
};

enum class AST_STMT_TYPES {
    ASSIGNMENT = 0,
    DEREF_PTR_ASSIGNMENT,
    FUNCTION_CALL,
    IF_THEN_ELSE,
    WHILE_LOOP,
    PASS
};

class AST_STMT {
public:
    AST_STMT_TYPES stmt_type;
    std::unique_ptr<AST_STMT> next; // Linked list of statements

    // Union types replaced with std::variant for type safety
    std::variant<AST_STMT_ASSIGNMENT, AST_STMT_FUNCTION_CALL, AST_STMT_WHILE_LOOP, AST_STMT_IF_THEN_ELSE> types;
};

enum class AST_BINARY_EXPR_OPERATORS {
    PLUS = 0,
    MINUS,
    MULT,
    POWER,
    MOD,
    DIV,
    EQUAL,
    NOT_EQUAL,
    LT,
    LTE,
    GT,
    GTE,
    IS,
    IN
};

enum class AST_UNARY_EXPR_TYPES {
    DEREF_PTR = 0,
    ADDRESS_OF_VAR,
    VARIABLE_ACCESS,
    INT_LITERAL,
    REAL_LITERAL,
    STR_LITERAL,
    TRUE_LITERAL,
    FALSE_LITERAL,
    NONE_LITERAL
};

class AST_UNARY_EXPR {
public:
    std::variant<std::string> types; // Here assuming all types are strings for simplicity
    AST_UNARY_EXPR_TYPES unary_expr_type;
};

class AST_BINARY_EXPR {
public:
    std::unique_ptr<AST_UNARY_EXPR> lhs;
    AST_BINARY_EXPR_OPERATORS op;
    std::unique_ptr<AST_UNARY_EXPR> rhs;
};

class AST_FUNCTION_CALL_EXPR {
public:
    std::string function_name;
    std::unique_ptr<AST_EXPR> param;
};

enum class AST_EXPR_TYPES {
    FUNCTION_CALL_EXPR = 0,
    UNARY_EXPR,
    BINARY_EXPR
};

class AST_EXPR {
public:
    AST_EXPR_TYPES expr_type;

    std::variant<std::unique_ptr<AST_FUNCTION_CALL_EXPR>, std::unique_ptr<AST_UNARY_EXPR>, std::unique_ptr<AST_BINARY_EXPR>> types;
};

std::unique_ptr<AST_STMT> ast_build(TokenQueue& tokens);
void ast_destroy(std::unique_ptr<AST_STMT>& ast);
