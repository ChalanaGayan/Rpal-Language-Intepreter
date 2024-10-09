#include "binaryOP.h"
#include <stdexcept>
#include <iostream>
using namespace std;
using std::invalid_argument;
using std::out_of_range;

string op(string type, string a, string b)
{
    int num_a = stoi(a);
    int num_b = stoi(b);

    int result;

    if (type == "+")
    {
        result = num_a + num_b;
    }
    else if (type == "-")
    {
        result = num_a - num_b;
    }
    else if (type == "*")
    {
        result = num_a * num_b;
    }
    else if (type == "/")
    {
        if (num_b == 0)
        {
            throw runtime_error("Division by zero is not allowed.");
        }
        result = num_a / num_b;
    }
    else
    {
        throw invalid_argument("Invalid operator: " + type);
    }
    return to_string(result);
}

string unop(string type, string a)
{
    if (type == "neg")
    {
        return to_string(-(stoi(a)));
    }
    else if (type == "not")
    {
        if (a == "true")
        {
            return "false";
        }
        else if (a == "false")
        {
            return "true";
        }
    }

    // If the 'type' is not valid or 'a' is not "true" or "false", throw an exception.
    throw invalid_argument("Invalid operator or operand: " + type + ", " + a);
}

string booleanops(string type, string a, string b)
{

    if (type == "or")
    {
        return (a == "true" || b == "true") ? "true" : "false";
    }
    if (type == "&")
    {
        return (a == "true" && b == "true") ? "true" : "false";
    }

    int num_a, num_b;
    try
    {
        num_a = stoi(a);
        num_b = stoi(b);
    }
    catch (const invalid_argument &e)
    {
        throw invalid_argument("Invalid numeric inputs.");
    }
    catch (const out_of_range &e)
    {
        throw out_of_range("Numeric inputs out of range.");
    }

    if (type == "eq")
    {
        return (num_a == num_b) ? "true" : "false";
    }
    else if (type == "ne")
    {
        return (num_a != num_b) ? "true" : "false";
    }
    else if (type == "gr")
    {
        return (num_a > num_b) ? "true" : "false";
    }
    else if (type == "ls")
    {
        return (num_a < num_b) ? "true" : "false";
    }
    else if (type == "ge")
    {
        return (num_a >= num_b) ? "true" : "false";
    }
    else if (type == "le")
    {
        return (num_a <= num_b) ? "true" : "false";
    }
    else
    {
        throw invalid_argument("Invalid operator: " + type);
    }
}
