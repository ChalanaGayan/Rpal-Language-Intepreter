#ifndef CUSTOMLEXER_H
#define CUSTOMLEXER_H

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include "Token.h"

using namespace std;

unordered_set<string> custom_keywords = {"let", "where", "within", "aug", "fn", "in"};
unordered_set<string> custom_operators = {"and", "or", "not", "gr", "ge", "ls", "le", "eq", "ne"};
unordered_set<string> custom_booleanValues = {"true", "false"};

class CustomLexer
{
public:
    // Constructor
    CustomLexer(const string &input) : input(input), currentPos(0) {}

    // Get the next token
    Token getNextToken()
    {
        skipWhitespace();

        if (currentPos >= input.length())
        {
            // Check if it is the last empty line or end of input
            if (currentPos == input.length())
                return {tokenType::END_OF_FILE, ""};
            else
                return {tokenType::DELIMITER, ""};
        }

        char currentChar = input[currentPos++];

        if (isalpha(currentChar))
        {
            stringstream stringStream;
            stringStream << currentChar;
            while (currentPos < input.length() &&
                   (isalnum(input[currentPos]) || input[currentPos] == '_'))
            {
                stringStream << input[currentPos++];
            }
            string identifier = stringStream.str();

            // Check if the identifier is a keyword
            if (custom_keywords.count(identifier) > 0)
            {
                return {tokenType::KEYWORD, identifier};
            }
            else if (custom_operators.count(identifier) > 0)
            {
                return {tokenType::OPERATOR, identifier};
            }
            else if (custom_booleanValues.count(identifier) > 0)
            {
                if (identifier == "true")
                    return {tokenType::INTEGER, "1"};
                else
                    return {tokenType::INTEGER, "0"};
            }

            return {tokenType::IDENTIFIER, identifier};
        }
        else if (isdigit(currentChar))
        {
            stringstream stringStream;
            stringStream << currentChar;
            while (currentPos < input.length() && isdigit(input[currentPos]))
            {
                stringStream << input[currentPos++];
            }
            string number = stringStream.str();
            return {tokenType::INTEGER, number};
        }
        else if (currentChar == '/')
        {
            if (currentPos < input.length() && input[currentPos] == '/')
            {
                // Skip single-line comment
                while (currentPos < input.length() && input[currentPos] != '\n')
                {
                    currentPos++;
                }
                // Recursively call getNextToken to get the next valid token
                return getNextToken();
            }
            else if (isOperatorSymbol(currentChar))
            {
                stringstream stringStream;
                stringStream << currentChar;
                while (currentPos < input.length() && isOperatorSymbol(input[currentPos]))
                {
                    stringStream << input[currentPos++];
                }
                string op = stringStream.str();
                return {tokenType::OPERATOR, op};
            }
            else
            {
                cerr << "Error: Unknown token encountered" << endl;
                return {tokenType::END_OF_FILE, ""};
            }
        }
        else if (isOperatorSymbol(currentChar))
        {
            stringstream stringStream;
            stringStream << currentChar;

            if (stringStream.str() == ",")
            {
                return {tokenType::OPERATOR, stringStream.str()};
            }

            while (currentPos < input.length() && isOperatorSymbol(input[currentPos]))
            {
                stringStream << input[currentPos++];
            }
            string op = stringStream.str();
            return {tokenType::OPERATOR, op};
        }
        else if (currentChar == '\'' || currentChar == '"')
        {
            bool isSingleQuote = currentChar == '\'';

            stringstream stringStream;
            while (currentPos < input.length())
            {
                currentChar = input[currentPos++];
                if (currentChar == '\'' && isSingleQuote || currentChar == '"' && !isSingleQuote)
                {
                    break;
                }
                else if (currentChar == '\\')
                {
                    currentChar = input[currentPos++];
                    switch (currentChar)
                    {
                    case 't':
                        stringStream << '\t';
                        break;
                    case 'n':
                        stringStream << '\n';
                        break;
                    case '\\':
                        stringStream << '\\';
                        break;
                    case '\'':
                        stringStream << '\'';
                        break;
                    default:
                        stringStream << '\\' << currentChar;
                        break;
                    }
                }
                else
                {
                    stringStream << currentChar;
                }
            }
            string str = stringStream.str();
            return {tokenType::STRING, str};
        }
        else if (currentChar == '(' || currentChar == ')')
        {
            string delimiter(1, currentChar);
            return {tokenType::DELIMITER, delimiter};
        }
        else
        {
            cerr << "Error: Unknown token encountered" << endl;
            return {tokenType::END_OF_FILE, ""};
        }
    }

private:
    // Skip whitespace in input stream
    void skipWhitespace()
    {
        while (currentPos < input.length() && isspace(input[currentPos]))
        {
            currentPos++;
        }
    }

    // Return whether a character is an operator symbol
    bool isOperatorSymbol(char c)
    {
        static const string operators = "+-*<>&.@/:=~|$!#%^_[}{?,";
        return operators.find(c) != string::npos;
    }

private:
    string input;
    size_t currentPos;
};

#endif // CUSTOMLEXER_H
