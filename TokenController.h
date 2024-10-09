#ifndef TOKENCONTROLLER_H
#define TOKENCONTROLLER_H

#include <vector>
#include "LexicalAnalyzer.h"
using namespace std;

// class TokenController manages tokens during parsing and implements the singleton pattern
class TokenController
{
private:
    static TokenController instance; // Instance
    vector<Token> tokens;            // Token vector
    int currentPos;                  // Current position in the token vector
    CustomLexer *lexer;              // Pointer to the lexer

    TokenController() {} // Private constructor

    ~TokenController() {} // Private destructor

    TokenController(const TokenController &) = delete; // Delete copy constructor
    TokenController &operator=(const TokenController &) = delete;

    // Store tokens in a vector until the end of file token is encountered retireved from the lexer
    void setTokens()
    {
        Token token;
        do
        {
            token = lexer->getNextToken();
            tokens.push_back(token);
        } while (token.type != tokenType::END_OF_FILE);
    }

public:
    // Returns the instance of the TokenController class.
    static TokenController &getInstance()
    {
        return instance;
    }

    // Set the lexer and initialize the tokens vector
    void setLexer(CustomLexer &lexer)
    {
        this->lexer = &lexer;
        setTokens();
        currentPos = 0;
    }

    // Returns a reference to the current token at the top of the tokens vector.
    Token &top()
    {
        return tokens[currentPos];
    }

    // Pop the current token from the tokens vector and return it
    Token &pop()
    {
        return tokens[currentPos++];
    }

    // Reset current position
    void reset()
    {
        currentPos = 0;
    }

    // Destroy the instance
    static void destroyInstance()
    {
        instance.lexer = nullptr;
        instance.tokens.clear();
    }
};

TokenController TokenController::instance; // Initializes static instance

#endif // TOKCONTROLLER_H