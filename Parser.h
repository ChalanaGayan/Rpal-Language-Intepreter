#ifndef RPAL_PARSER_H
#define RPAL_PARSER_H

#include <vector>
#include "Token.h"
#include "LexicalAnalyzer.h"
#include "TokenController.h"
#include "Tree.h"
#include "TreeNode.h"
using namespace std;

void E();
void Ew();
void T();
void Ta();
void Tc();
void B();
void Bt();
void Bs();
void Bp();
void A();
void At();
void Af();
void Ap();
void R();
void Rn();
void D();
void Da();
void Dr();
void Db();
void Vb();
void Vl();

// parser
class Parser
{
public:
    static vector<TreeNode *> NodeOfStack;

    static void parse()
    {
        TokenController &tokenController = TokenController::getInstance();
        Token token = tokenController.top();

        // if input token equals end of file token
        if (token.type == tokenType::END_OF_FILE)
        {
            return;
        }
        else
        {
            E(); // initialize with E()

            // if next token is end of file token
            if (tokenController.top().type == tokenType::END_OF_FILE)
            {
                // set root of ast to last node in NodeOfStack
                Tree::getInstance().setASTRoot(Parser::NodeOfStack.back());
                return;
            }
            else
            {
                throw runtime_error("Syntax Error: end of file expected");
            }
        }
    }
};

vector<TreeNode *> Parser::NodeOfStack;

void build_tree(const string &label, const int &num, const bool isLeaf, const string &value = "")
{
    TreeNode *node;

    if (isLeaf)
    {
        node = new LeafNode(label, value);
    }
    else
    {
        node = new InternalNode(label);
    }
    for (int i = 0; i < num; i++)
    {
        node->addChild(Parser::NodeOfStack.back());
        Parser::NodeOfStack.pop_back();
    }
    node->reverseChildren();

    Parser::NodeOfStack.push_back(node);
}

// Parses the expression (E).
// This function is the entry point for parsing expressions.
void E()
{
    TokenController &tokenController = TokenController::getInstance();

    // If the current token is "let"
    if (tokenController.top().value == "let")
    {
        tokenController.pop();
        D(); // Parse declaration
        // If the next token is "in"
        if (tokenController.top().value == "in")
        {
            tokenController.pop();
            E(); // Parse nested expression
        }
        else
        {
            throw runtime_error("Syntax Error: 'in' expected");
        }
        // Construct a "let" node with 2 children
        build_tree("let", 2, false);
    }
    // If the current token is "fn"
    else if (tokenController.top().value == "fn")
    {
        tokenController.pop();
        int n = 0;

        // Process identifiers until a non-identifier token is encountered
        while (tokenController.top().type == tokenType::IDENTIFIER)
        {
            Vb(); // Parse variable binding
            n++;
        }

        if (n == 0)
        {
            throw runtime_error("Syntax Error: at least one identifier expected");
        }

        // If the next token is "."
        if (tokenController.top().value == ".")
        {
            tokenController.pop();
            E(); // Parse nested expression
        }
        else
        {
            throw runtime_error("Syntax Error: '.' expected");
        }

        // Construct a "lambda" node with n+1 children
        build_tree("lambda", n + 1, false);
    }
    else
    {
        Ew(); // Parse Ew (Expression with "where" clause)
    }
}

// Parses the expression with a "where" clause (Ew).
// This function handles expressions followed by a "where" clause.
void Ew()
{
    TokenController &tokenController = TokenController::getInstance();
    T(); // Parse term

    // If the next token is "where"
    if (tokenController.top().value == "where")
    {
        tokenController.pop();
        Dr();                          // Parse declaration list
        build_tree("where", 2, false); // Construct a "where" node with 2 children
    }
}

// Parses a term (T).
// This function handles parsing terms and their combination with commas.
void T()
{
    TokenController &tokenController = TokenController::getInstance();
    Ta(); // Parse atomic term
    int n = 0;

    // Process additional atomic terms separated by commas
    while (tokenController.top().value == ",")
    {
        tokenController.pop();
        Ta(); // Parse atomic term
        n++;
    }

    if (n > 0)
    {
        build_tree("tau", n + 1, false); // Construct a "tau" node with n+1 children
    }
}

// Parses an atomic term (Ta).
// This function handles parsing atomic terms and their combination with "aug" clauses.
void Ta()
{
    TokenController &tokenController = TokenController::getInstance();
    Tc(); // Parse term construction
    // Process additional term constructions separated by "aug" clauses
    while (tokenController.top().value == "aug")
    {
        tokenController.pop();
        Tc();                        // Parse term construction
        build_tree("aug", 2, false); // Construct an "aug" node with 2 children
    }
}

// Parses a term construction (Tc).
// This function handles parsing term constructions and their combination with "->" and "|" clauses.
void Tc()
{
    TokenController &tokenController = TokenController::getInstance();
    B(); // Parse boolean expression

    // If the next token is "->"
    if (tokenController.top().value == "->")
    {
        tokenController.pop();
        Tc(); // Parse nested term construction

        // If the next token is "|"
        if (tokenController.top().value == "|")
        {
            tokenController.pop();
            Tc();                       // Parse nested term construction
            build_tree("->", 3, false); // Construct a "->" node with 3 children
        }
        else
        {
            throw runtime_error("Syntax Error: '|' expected");
        }
    }
}

// Parses a boolean expression (B).
// This function handles parsing boolean expressions and their combination with "or" clauses.
void B()
{
    TokenController &tokenController = TokenController::getInstance();
    Bt(); // Parse boolean term

    // Process additional boolean terms separated by "or" clauses
    while (tokenController.top().value == "or")
    {
        tokenController.pop();
        Bt();                       // Parse boolean term
        build_tree("or", 2, false); // Construct an "or" node with 2 children
    }
}

// Parses a boolean term (Bt).
// This function handles parsing boolean terms and their combination with "&" clauses.
void Bt()
{
    TokenController &tokenController = TokenController::getInstance();
    Bs(); // Parse boolean factor

    // Process additional boolean factors separated by "&" clauses
    while (tokenController.top().value == "&")
    {
        tokenController.pop();
        Bs();                      // Parse boolean factor
        build_tree("&", 2, false); // Construct an "&" node with 2 children
    }
}

// Parses a boolean factor (Bs).
// This function handles parsing boolean factors and negations.
void Bs()
{
    TokenController &tokenController = TokenController::getInstance();
    if (tokenController.top().value == "not")
    {
        tokenController.pop();
        Bp();                        // Parse boolean primary
        build_tree("not", 1, false); // Construct a "not" node with 1 child
    }
    else
    {
        Bp(); // Parse boolean primary
    }
}

// Parses a boolean primary (Bp).
// This function handles parsing boolean primaries and their comparison operations.
void Bp()
{
    TokenController &tokenController = TokenController::getInstance();
    A(); // Parse arithmetic expression

    // Check for comparison operators
    if (tokenController.top().value == "gr" || tokenController.top().value == ">")
    {
        tokenController.pop();
        A();                        // Parse another arithmetic expression
        build_tree("gr", 2, false); // Construct a "gr" node with 2 children
    }
    else if (tokenController.top().value == "ge" || tokenController.top().value == ">=")
    {
        tokenController.pop();
        A();                        // Parse another arithmetic expression
        build_tree("ge", 2, false); // Construct a "ge" node with 2 children
    }
    else if (tokenController.top().value == "ls" || tokenController.top().value == "<")
    {
        tokenController.pop();
        A();                        // Parse another arithmetic expression
        build_tree("ls", 2, false); // Construct a "ls" node with 2 children
    }
    else if (tokenController.top().value == "le" || tokenController.top().value == "<=")
    {
        tokenController.pop();
        A();                        // Parse another arithmetic expression
        build_tree("le", 2, false); // Construct a "le" node with 2 children
    }
    else if (tokenController.top().value == "eq" || tokenController.top().value == "=")
    {
        tokenController.pop();
        A();                        // Parse another arithmetic expression
        build_tree("eq", 2, false); // Construct an "eq" node with 2 children
    }
    else if (tokenController.top().value == "ne" || tokenController.top().value == "!=")
    {
        tokenController.pop();
        A();                        // Parse another arithmetic expression
        build_tree("ne", 2, false); // Construct a "ne" node with 2 children
    }
}

// Parses an arithmetic expression (A).
// This function handles parsing arithmetic expressions and their combination with addition and subtraction operators.
void A()
{
    TokenController &tokenController = TokenController::getInstance();

    // Check for unary plus operator
    if (tokenController.top().value == "+")
    {
        tokenController.pop();
        At(); // Parse term
    }
    // Check for unary minus operator
    else if (tokenController.top().value == "-")
    {
        tokenController.pop();
        At();                        // Parse term
        build_tree("neg", 1, false); // Construct a "neg" node with 1 child
    }
    else
    {
        At(); // Parse term
    }

    // Process addition and subtraction operators
    while (tokenController.top().value == "+" || tokenController.top().value == "-")
    {
        if (tokenController.top().value == "+")
        {
            tokenController.pop();
            At();                      // Parse term
            build_tree("+", 2, false); // Construct a "+" node with 2 children
        }
        else if (tokenController.top().value == "-")
        {
            tokenController.pop();
            At();                      // Parse term
            build_tree("-", 2, false); // Construct a "-" node with 2 children
        }
    }
}

// Parses a term (At).
// This function handles parsing terms and their combination with multiplication and division operators.
void At()
{
    TokenController &tokenController = TokenController::getInstance();
    Af(); // Parse atomic formula

    // Process multiplication and division operators
    while (tokenController.top().value == "*" || tokenController.top().value == "/")
    {
        if (tokenController.top().value == "*")
        {
            tokenController.pop();
            Af();                      // Parse atomic formula
            build_tree("*", 2, false); // Construct a "*" node with 2 children
        }
        else if (tokenController.top().value == "/")
        {
            tokenController.pop();
            Af();                      // Parse atomic formula
            build_tree("/", 2, false); // Construct a "/" node with 2 children
        }
    }
}

// Parses an atomic formula (Af).
// This function handles parsing atomic formulas and their combination with exponentiation operators.
void Af()
{
    TokenController &tokenController = TokenController::getInstance();
    Ap(); // Parse application

    // Process exponentiation operator
    while (tokenController.top().value == "**")
    {
        tokenController.pop();
        Ap();                       // Parse application
        build_tree("**", 2, false); // Construct a "**" node with 2 children
    }
}

// Parses an application (Ap).
// This function handles parsing function applications.
void Ap()
{
    TokenController &tokenController = TokenController::getInstance();
    R(); // Parse basic expression

    // Process function application operator
    while (tokenController.top().value == "@")
    {
        tokenController.pop();

        // If the next token is an identifier
        if (tokenController.top().type == tokenType::IDENTIFIER)
        {
            Token token = tokenController.pop();
            build_tree("identifier", 0, true, token.value); // Construct an identifier node
        }
        else
        {
            throw runtime_error("Syntax Error: Identifier expected");
        }

        R();                       // Parse basic expression
        build_tree("@", 3, false); // Construct an "@" node with 3 children
    }
}

// Parses a basic expression (R).
// This function handles parsing basic expressions and their combinations.
void R()
{
    TokenController &tokenController = TokenController::getInstance();
    Rn(); // Parse basic factor

    Token top = tokenController.top();
    while (top.type == tokenType::IDENTIFIER || top.type == tokenType::INTEGER || top.type == tokenType::STRING || top.value == "true" || top.value == "false" || top.value == "nil" || top.value == "(" || top.value == "dummy")
    {
        Rn(); // Parse basic factor
        top = tokenController.top();
        build_tree("gamma", 2, false); // Construct a "gamma" node with 2 children
    }
}

// Parses a basic factor (Rn).
// This function handles parsing basic factors such as identifiers, integers, strings, etc.
void Rn()
{
    TokenController &tokenController = TokenController::getInstance();
    Token top = tokenController.top();

    if (top.type == tokenType::IDENTIFIER)
    {
        // Parse identifier
        Token token = tokenController.pop();
        build_tree("identifier", 0, true, token.value); // Construct an identifier node
    }
    else if (top.type == tokenType::INTEGER)
    {
        // Parse integer
        Token token = tokenController.pop();
        build_tree("integer", 0, true, token.value); // Construct an integer node
    }
    else if (top.type == tokenType::STRING)
    {
        // Parse string
        Token token = tokenController.pop();
        build_tree("string", 0, true, token.value); // Construct a string node
    }
    else if (top.value == "true")
    {
        // Parse true
        tokenController.pop();
        build_tree("true", 0, true); // Construct a "true" node
    }
    else if (top.value == "false")
    {
        // Parse false
        tokenController.pop();
        build_tree("false", 0, true); // Construct a "false" node
    }
    else if (top.value == "nil")
    {
        // Parse nil
        tokenController.pop();
        build_tree("nil", 0, true); // Construct a "nil" node
    }
    else if (top.value == "(")
    {
        tokenController.pop();
        E(); // Parse expression

        if (tokenController.top().value == ")")
        {
            tokenController.pop();
        }
        else
        {
            throw runtime_error("Syntax Error: ')' expected");
        }
    }
    else if (top.value == "dummy")
    {
        // Parse dummy
        tokenController.pop();
        build_tree("dummy", 0, true); // Construct a "dummy" node
    }
    else
    {
        throw runtime_error("Syntax Error: Identifier, Integer, String, 'true', 'false', 'nil', '(', 'dummy' expected\ngot: " + top.value);
    }
}

// Parses a declaration (D).
// This function handles parsing declarations.
void D()
{
    TokenController &tokenController = TokenController::getInstance();
    Da(); // Parse declaration(s) followed by "within"

    while (tokenController.top().value == "within")
    {
        tokenController.pop();
        D();                            // Parse declaration(s)
        build_tree("within", 2, false); // Construct a "within" node with 2 children
    }
}

// Parses a declaration (Da).
// This function handles parsing declaration(s) followed by "and".
void Da()
{
    TokenController &tokenController = TokenController::getInstance();
    Dr(); // Parse recursive declaration(s)
    int n = 0;

    while (tokenController.top().value == "and")
    {
        tokenController.pop();
        Dr(); // Parse recursive declaration(s)
        n++;
    }
    if (n > 0)
    {
        build_tree("and", n + 1, false); // Construct an "and" node with n+1 children
    }
}

// Parses a recursive declaration (Dr).
// This function handles parsing recursive declaration(s) preceded by "rec".
void Dr()
{
    TokenController &tokenController = TokenController::getInstance();

    if (tokenController.top().value == "rec")
    {
        tokenController.pop();
        Db();                        // Parse basic declaration
        build_tree("rec", 1, false); // Construct a "rec" node with 1 child
    }
    else
    {
        Db(); // Parse basic declaration
    }
}

// Parses a basic declaration (Db).
// This function handles parsing basic declaration(s) defined by identifiers and expressions.
void Db()
{
    TokenController &tokenController = TokenController::getInstance();

    if (tokenController.top().value == "(")
    {
        tokenController.pop();
        D(); // Parse declaration
        if (tokenController.top().value == ")")
        {
            tokenController.pop();
        }
        else
        {
            throw runtime_error("Syntax Error: ')' expected");
        }
    }
    else if (tokenController.top().type == tokenType::IDENTIFIER)
    {
        // Parse identifier
        Token token = tokenController.pop();
        build_tree("identifier", 0, true, token.value);

        if (tokenController.top().value == ",")
        {
            tokenController.pop();
            Vl(); // Parse variable list

            if (tokenController.top().value == "=")
            {
                tokenController.pop();
                E();                       // Parse expression
                build_tree("=", 2, false); // Construct an "=" node with 2 children
            }
            else
            {
                throw runtime_error("Syntax Error: '=' expected");
            }
        }
        else
        {
            int n = 0;

            while (tokenController.top().value != "=" && tokenController.top().type == tokenType::IDENTIFIER)
            {
                Vb(); // Parse variable bound
                n++;
            }

            if (tokenController.top().value == "(")
            {
                Vb(); // Parse variable bound
                n++;
            }

            if (n == 0 && tokenController.top().value == "=")
            {
                tokenController.pop();
                E();                       // Parse expression
                build_tree("=", 2, false); // Construct an "=" node with 2 children
            }
            else if (n != 0 && tokenController.top().value == "=")
            {
                tokenController.pop();
                E();                                  // Parse expression
                build_tree("fcn_form", n + 2, false); // Construct a "fcn_form" node with n+2 children
            }
            else
            {
                throw runtime_error("Syntax Error: '=' expected");
            }
        }
    }
    else
    {
        throw runtime_error("Syntax Error: '(' or Identifier expected");
    }
}

// Parses a variable bound (Vb).
// This function handles parsing variable bounds, which can be identifiers or parenthesized expressions.
void Vb()
{
    TokenController &tokenController = TokenController::getInstance();

    if (tokenController.top().type == tokenType::IDENTIFIER)
    {
        // Parse identifier
        Token token = tokenController.pop();
        build_tree("identifier", 0, true, token.value); // Construct an identifier node
    }
    else if (tokenController.top().value == "(")
    {
        tokenController.pop();

        if (tokenController.top().value == ")")
        {
            tokenController.pop();
            build_tree("()", 0, true); // Construct an "()" node
        }
        else if (tokenController.top().type == tokenType::IDENTIFIER)
        {
            // Parse identifier
            Token token = tokenController.pop();
            build_tree("identifier", 0, true, token.value); // Construct an identifier node

            if (tokenController.top().value == ",")
            {
                tokenController.pop();
                Vl(); // Parse variable list
            }

            if (tokenController.top().value == ")")
            {
                tokenController.pop();
            }
            else
            {
                throw runtime_error("Syntax Error: ')' expected");
            }
        }
        else
        {
            throw runtime_error("Syntax Error: Identifier or ')' expected");
        }
    }
    else
    {
        throw runtime_error("Syntax Error: Identifier or '(' expected");
    }
}

// Parses a variable list (Vl).
// This function handles parsing comma-separated lists of identifiers.
void Vl()
{
    TokenController &tokenController = TokenController::getInstance();

    if (tokenController.top().type == tokenType::IDENTIFIER)
    {
        // Parse identifier
        Token token = tokenController.pop();
        build_tree("identifier", 0, true, token.value); // Construct an identifier node

        int n = 2;
        while (tokenController.top().value == ",")
        {
            tokenController.pop();
            token = tokenController.pop();
            build_tree("identifier", 0, true, token.value); // Construct an identifier node
            n++;
        }

        build_tree(",", n, false); // Construct a "," node with n children
    }
    else
    {
        throw runtime_error("Syntax Error: Identifier expected");
    }
}

#endif // RPAL_PARSER_H