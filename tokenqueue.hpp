#pragma once

#include <iostream> 
#include <cstdbool> 
#include "token.hpp"   

class TokenNode
{
public:
    Token token;         
    char* value;
    TokenNode* next;

    TokenNode() : value(nullptr), next(nullptr) {} // Constructor for initialization
};

class TokenQueue
{
public:
    TokenNode* head;
    TokenNode* tail;

    TokenQueue() : head(nullptr), tail(nullptr) {} // Constructor for initialization
};

// Functions
TokenQueue* tokenqueue_create();
void        tokenqueue_destroy(TokenQueue* tokens);

void tokenqueue_enqueue(TokenQueue* tokens, Token token, char* value);
void tokenqueue_dequeue(TokenQueue* tokens);
bool tokenqueue_empty(const TokenQueue* tokens);
Token tokenqueue_peekToken(const TokenQueue* tokens);
char* tokenqueue_peekValue(const TokenQueue* tokens);
void tokenqueue_print(const TokenQueue* tokens);

TokenQueue* tokenqueue_duplicate(const TokenQueue* tokens);
