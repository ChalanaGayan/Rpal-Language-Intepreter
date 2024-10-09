#ifndef TOKEN_H
#define TOKEN_H

#include <string>
using namespace std;

// enumerate token types
enum tokenType
{
    IDENTIFIER,
    INTEGER,
    STRING,
    OPERATOR,
    DELIMITER,
    KEYWORD,
    END_OF_FILE
};

struct Token
{
    tokenType type;
    string value;
};

#endif // TOKEN_H