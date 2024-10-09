#ifndef CSE_H
#define CSE_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include "Tree.h"
#include "BOP/binaryOP.h"

using namespace std;

// enumerator for node types in CSE machine
enum class ObjectType : int
{
    LAMBDA,
    IDENTIFIER,
    INTEGER,
    STRING,
    GAMMA,
    OPERATOR,
    BETA,
    EETA,
    DELTA,
    TAU,
    ENV,
    LIST,
    BOOLEAN
};

vector<string> built_in_functions = {"Print", "print", "Order", "Y*", "Conc", "Stem", "Stern", "Isinteger", "Isstring", "Istuple", "Isempty", "dummy", "ItoS"};

bool isOperator(const string &label);

class CSENode
{
private:
    // General node properties
    ObjectType nodeType;
    string nodeValue;

    // lambda and eeta nodes
    int env{};
    int csIndex{}; // for delta, tau, eeta, lambda nodes
    vector<string> boundVariables;
    vector<CSENode> listElements;
    bool isSingleBoundVar = true;

public:
    CSENode() = default;

    // constructors
    // lambda (in stack) and eeta nodes
    CSENode(ObjectType nodeType, string nodeValue, int csIndex, int env)
    {
        this->nodeType = nodeType;
        this->nodeValue = move(nodeValue);
        this->csIndex = csIndex;
        this->env = env;
    }

    // lambda (in control structure) nodes
    CSENode(ObjectType nodeType, string nodeValue, int csIndex)
    {
        this->nodeType = nodeType;
        this->nodeValue = move(nodeValue);
        this->csIndex = csIndex;
    }

    // lambda (in control structure) nodes with bound variables
    CSENode(ObjectType nodeType, int csIndex, vector<string> boundVariables)
    {
        isSingleBoundVar = false;
        this->nodeType = nodeType;
        this->csIndex = csIndex;
        this->boundVariables = move(boundVariables);
    }

    // lambda (in stack) nodes with bound variables
    CSENode(ObjectType nodeType, int csIndex, vector<string> boundVariables, int env)
    {
        isSingleBoundVar = false;
        this->nodeType = nodeType;
        this->nodeValue = move(nodeValue);
        this->csIndex = csIndex;
        this->env = env;
        this->boundVariables = move(boundVariables);
    }

    // other nodes
    CSENode(ObjectType nodeType, string nodeValue)
    {
        this->nodeType = nodeType;
        this->nodeValue = move(nodeValue);
    }

    // lists elem_1
    CSENode(ObjectType nodeType, vector<CSENode> listElements)
    {
        this->nodeType = nodeType;
        this->listElements = move(listElements);
    }

    // getters
    ObjectType get_NodeType() const { return nodeType; }

    string get_nodeValue() const { return nodeValue; }

    int get_ENV() const { return env; }

    int get_CSIndex() const { return csIndex; }

    bool get_IsSingleBoundVar() const { return isSingleBoundVar; }

    vector<string> get_varList() const { return boundVariables; }

    vector<CSENode> get_ListElements() const { return listElements; }

    // setter
    CSENode set_ENV(int newEnv)
    {
        this->env = newEnv;
        return *this;
    }
};

class ControlStructure
{

private:
    int csIndex;
    vector<CSENode> nodes;

public:
    // Constructor with empty nodes
    explicit ControlStructure(int csIndex) { this->csIndex = csIndex; }

    // add node to control structure
    void addNode(CSENode node) { nodes.push_back(node); }

    // Getters
    int get_CSIndex() const { return csIndex; }

    // pop the last node in the control structure
    void popLastNode() { nodes.pop_back(); }

    // pop and return the last node in the control structure
    CSENode returnLastNode()
    {
        CSENode node = nodes.back();
        nodes.pop_back();
        return node;
    }

    // push another control structure to the current control structure
    void addNewCS(const ControlStructure &cs)
    {
        for (auto &node : cs.nodes)
        {
            nodes.push_back(node);
        }
    }
};

class Stack
{
private:
    vector<CSENode> nodes;

public:
    // constructor with empty nodes
    Stack() = default;

    // add node
    void addNode(const CSENode &node) { nodes.push_back(node); }

    // pop the last node
    void popLastNode() { nodes.pop_back(); }

    // pop and return the last node
    CSENode returnLastNode()
    {
        CSENode node = nodes.back();
        nodes.pop_back();
        return node;
    }

    // length of the stack
    int length() const { return static_cast<int>(nodes.size()); }
};

class Env
{
private:
    unordered_map<string, CSENode> variables;
    unordered_map<string, CSENode> lambdas;
    unordered_map<string, vector<CSENode>> lists;
    Env *parentENV;

public:
    // constructor empty env
    Env() { parentENV = nullptr; }

    explicit Env(Env *parentENV) { this->parentENV = parentENV; }

    // add single variable to env
    void add_variable(const string &identifier, const CSENode &value) { variables[identifier] = value; }

    // add multiple variable to env
    void add_variables(const vector<string> &identifiers, const vector<CSENode> &values)
    {
        for (int i = 0; i < identifiers.size(); i++)
        {
            variables[identifiers[i]] = values[i];
        }
    }

    void add_List(const string &identifier, vector<CSENode> listElements) { lists[identifier] = move(listElements); }

    // add lambda to environment
    void add_Lambda(const string &identifier, const CSENode &lambda)
    {

        // check the node type and create new object
        if (lambda.get_NodeType() == ObjectType::LAMBDA)
        {
            if (lambda.get_IsSingleBoundVar())
            {
                lambdas[identifier] = CSENode(ObjectType::LAMBDA, lambda.get_nodeValue(), lambda.get_CSIndex(), lambda.get_ENV());
            }
            else
            {
                lambdas[identifier] = CSENode(ObjectType::LAMBDA, lambda.get_CSIndex(), lambda.get_varList(), lambda.get_ENV());
            }
        }
        else if (lambda.get_NodeType() == ObjectType::EETA)
        {
            if (lambda.get_IsSingleBoundVar())
            {
                lambdas[identifier] = CSENode(ObjectType::EETA, lambda.get_nodeValue(), lambda.get_CSIndex(), lambda.get_ENV());
            }
            else
            {
                lambdas[identifier] = CSENode(ObjectType::EETA, lambda.get_CSIndex(), lambda.get_varList(), lambda.get_ENV());
            }
        }
        else
        {
            throw runtime_error("Invalid lambda node type");
        }
    }

    // find variable from environment
    CSENode findVariable(const string &identifier)
    {
        if (variables.find(identifier) != variables.end())
        {
            return variables[identifier];
        }
        else if (parentENV != nullptr)
        {
            return parentENV->findVariable(identifier);
        }
        else
        {
            throw runtime_error("Identifier: " + identifier + " not found");
        }
    }

    // find lambda from environment
    CSENode findLambda(const string &identifier)
    {
        if (lambdas.find(identifier) != lambdas.end())
        {
            return lambdas[identifier];
        }
        else if (parentENV != nullptr)
        {
            return parentENV->findLambda(identifier);
        }
        else
        {
            throw runtime_error("Identifier: " + identifier + " not found");
        }
    }

    // find list from environment
    vector<CSENode> findList(const string &identifier)
    {
        if (lists.find(identifier) != lists.end())
        {
            return lists[identifier];
        }
        else if (parentENV != nullptr)
        {
            return parentENV->findList(identifier);
        } // call get list recursively
        else
        {
            throw runtime_error("Identifier: " + identifier + " not found");
        }
    }
};

class CSE
{
private:
    int next_env = 0;
    int nextCS = -1;

    vector<ControlStructure *> controlStructures;
    ControlStructure cse_machine = ControlStructure(-1);
    Stack stack = Stack();
    vector<int> env_stack = vector<int>();
    unordered_map<int, Env *> envs = unordered_map<int, Env *>();

public:
    // constructor with empty control structures and stack
    CSE() = default;

    // create control structures
    void createCS(TreeNode *root, ControlStructure *current_cs = nullptr, int currentCSIndex = -1)
    {
        ControlStructure *cs;

        if (nextCS == -1)
        {
            nextCS++;
            cs = new ControlStructure(nextCS++);
            controlStructures.push_back(cs);
            currentCSIndex = 0;
        }
        else
        {
            cs = current_cs;
        }

        if (root->getLabel() == "lambda")
        {
            CSENode *lambda;
            if (root->getChildren()[0]->getLabel() == ",")
            {
                vector<string> vars;
                for (auto &child : root->getChildren()[0]->getChildren())
                {
                    vars.push_back(child->getValue());
                }

                lambda = new CSENode(ObjectType::LAMBDA, nextCS, vars);
            }
            else
            {
                string var = root->getChildren()[0]->getValue();
                lambda = new CSENode(ObjectType::LAMBDA, var, nextCS);
            }

            cs->addNode(*lambda);

            auto *newCS = new ControlStructure(nextCS);
            controlStructures.push_back(newCS);
            createCS(root->getChildren()[1], newCS, nextCS++);
        }

        else if (root->getLabel() == "tau")
        {
            auto *tau = new CSENode(ObjectType::TAU, to_string(root->getChildren().size()));
            cs->addNode(*tau);

            for (auto &child : root->getChildren())
            {
                createCS(child, cs, currentCSIndex);
            }
        }

        else if (root->getLabel() == "->")
        {
            int thenCSIndex = nextCS++;
            int elseCSIndex = nextCS++;

            // Create new CSs
            cs->addNode(*(new CSENode(ObjectType::DELTA, to_string(thenCSIndex))));
            cs->addNode(*(new CSENode(ObjectType::DELTA, to_string(elseCSIndex))));
            cs->addNode(*(new CSENode(ObjectType::BETA, "")));

            auto *thenCS = new ControlStructure(thenCSIndex);
            controlStructures.push_back(thenCS);
            createCS(root->getChildren()[1], thenCS, thenCSIndex);

            auto *elseCS = new ControlStructure(elseCSIndex);
            controlStructures.push_back(elseCS);
            createCS(root->getChildren()[2], elseCS, elseCSIndex);

            createCS(root->getChildren()[0], cs, currentCSIndex);
        }
        else if (isOperator(root->getLabel()))
        {
            cs->addNode(*(new CSENode(ObjectType::OPERATOR, root->getLabel())));

            for (auto &child : root->getChildren())
            {
                createCS(child, cs, currentCSIndex);
            }
        }
        else if (root->getLabel() == "gamma")
        {
            cs->addNode(*(new CSENode(ObjectType::GAMMA, "")));

            for (auto &child : root->getChildren())
            {
                createCS(child, cs, currentCSIndex);
            }
        }
        else if (root->getLabel() == "identifier" || root->getLabel() == "integer" || root->getLabel() == "string")
        {
            string value = root->getValue();
            string type = root->getLabel();

            unordered_map<string, ObjectType> typeMap = {{"identifier", ObjectType::IDENTIFIER}, {"integer", ObjectType::INTEGER}, {"string", ObjectType::STRING}};

            // Check if the type is valid and get the corresponding ObjectType
            auto objectTypeIter = typeMap.find(type);

            if (objectTypeIter != typeMap.end())
            {
                // Create the CSENode with the correct ObjectType
                cs->addNode(CSENode(objectTypeIter->second, value));
            }
            else
            {
                throw runtime_error("Invalid leaf type: " + type);
            }
        }
        else
        {
            throw runtime_error("Invalid node type: " + root->getLabel() + "Value: " + root->getValue());
        }
    }

    void evaluate()
    {
        auto *e0 = new CSENode(ObjectType::ENV, "0");
        cse_machine.addNode(*e0);
        stack.addNode(*e0);
        env_stack.push_back(next_env++);
        envs[0] = new Env(nullptr);

        cse_machine.addNewCS(*controlStructures[0]);

        CSENode top = cse_machine.returnLastNode();

        while ((top.get_NodeType() != ObjectType::ENV) || (top.get_nodeValue() != "0"))
        {
            if (top.get_NodeType() == ObjectType::INTEGER || top.get_NodeType() == ObjectType::STRING)
            {
                stack.addNode(top);
                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::IDENTIFIER)
            {
                CSENode value;
                CSENode value_l;
                vector<CSENode> list;

                try
                {
                    value = envs[env_stack.back()]->findVariable(top.get_nodeValue());
                    stack.addNode(CSENode(value.get_NodeType(), value.get_nodeValue()));
                }
                catch (runtime_error &e)
                {
                    try
                    {
                        value_l = envs[env_stack.back()]->findLambda(top.get_nodeValue());
                        stack.addNode(value_l);
                    }
                    catch (runtime_error &e)
                    {
                        try
                        {
                            list = envs[env_stack.back()]->findList(top.get_nodeValue());
                            stack.addNode(CSENode(ObjectType::LIST, list));
                        }
                        catch (runtime_error &e)
                        {
                            // if node value is in built_in_functions add the node to the stack
                            if (find(built_in_functions.begin(), built_in_functions.end(),
                                     top.get_nodeValue()) !=
                                built_in_functions.end())
                            {
                                stack.addNode(top);
                            }
                            else if (top.get_nodeValue() == "nil")
                            {
                                stack.addNode(CSENode(ObjectType::LIST, vector<CSENode>()));
                            }
                            else
                            {
                                throw runtime_error("Variable not found: " + top.get_nodeValue());
                            }
                        }
                    }
                }

                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::LAMBDA)
            {
                int current_env = env_stack.back();
                stack.addNode(top.set_ENV(current_env));

                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::GAMMA)
            {
                CSENode top_of_stack = stack.returnLastNode();

                if (top_of_stack.get_NodeType() == ObjectType::LAMBDA)
                {
                    Env *new_env = new Env(envs[top_of_stack.get_ENV()]);
                    envs[next_env++] = new_env;

                    CSENode value = stack.returnLastNode();

                    if (value.get_NodeType() == ObjectType::LAMBDA || value.get_NodeType() == ObjectType::EETA)
                    {
                        new_env->add_Lambda(top_of_stack.get_nodeValue(), value);
                    }
                    else if (value.get_NodeType() == ObjectType::STRING || value.get_NodeType() == ObjectType::INTEGER)
                    {
                        new_env->add_variable(top_of_stack.get_nodeValue(), value);
                    }
                    else if (value.get_NodeType() == ObjectType::LIST && !top_of_stack.get_IsSingleBoundVar())
                    {
                        vector<string> var_list = top_of_stack.get_varList();
                        vector<CSENode> list_items = value.get_ListElements();

                        vector<string> list_var;
                        vector<CSENode> temp_list = vector<CSENode>();

                        int var_count = 0;

                        int list_element_count = 0;
                        bool creating_list = false;

                        for (const auto &i : list_items)
                        {
                            if (creating_list)
                            {
                                temp_list.push_back(i);
                                list_element_count--;

                                if (list_element_count == 0)
                                {
                                    new_env->add_List(var_list[var_count++], temp_list);
                                    temp_list = vector<CSENode>();
                                    creating_list = false;
                                }
                            }
                            else
                            {
                                if (i.get_NodeType() == ObjectType::LIST)
                                {
                                    list_element_count = stoi(i.get_nodeValue());
                                    if (list_element_count == 0)
                                    {
                                        new_env->add_List(var_list[var_count++], temp_list);
                                        temp_list = vector<CSENode>();
                                    }
                                    else
                                    {
                                        creating_list = true;
                                    }
                                }
                                else if (i.get_NodeType() == ObjectType::LAMBDA)
                                {
                                    new_env->add_Lambda(var_list[var_count++], i);
                                }
                                else
                                {
                                    new_env->add_variable(var_list[var_count++], i);
                                }
                            }
                        }

                        if (creating_list)
                        {
                            new_env->add_List(var_list[var_count], temp_list);
                        }
                    }
                    else if (value.get_NodeType() == ObjectType::LIST)
                    {
                        new_env->add_List(top_of_stack.get_nodeValue(), value.get_ListElements());
                    }
                    else
                    {
                        throw runtime_error("Invalid object for gamma: " + value.get_nodeValue());
                    }

                    env_stack.push_back(next_env - 1);
                    auto *env_obj = new CSENode(ObjectType::ENV, to_string(next_env - 1));
                    cse_machine.addNode(*env_obj);
                    stack.addNode(*env_obj);
                    cse_machine.addNewCS(*controlStructures[top_of_stack.get_CSIndex()]);
                }
                else if (top_of_stack.get_NodeType() == ObjectType::IDENTIFIER)
                {
                    // TODO: built-in functions should be handled here
                    string identifier = top_of_stack.get_nodeValue();

                    if (identifier == "Print" || identifier == "print")
                    {
                        CSENode value = stack.returnLastNode();
                        vector<CSENode> listElements = value.get_ListElements();

                        if (value.get_NodeType() == ObjectType::LIST)
                        {
                            cout << "(";

                            vector<int> count_stack;

                            for (int i = 0; i < value.get_ListElements().size(); i++)
                            {
                                if (listElements[i].get_NodeType() == ObjectType::LIST)
                                {
                                    count_stack.push_back(stoi(listElements[i].get_nodeValue()));
                                    cout << "(";
                                }
                                else
                                {
                                    cout << listElements[i].get_nodeValue();

                                    if (!count_stack.empty())
                                    {
                                        // reduce 1 from all elem_1 in count_stack
                                        for (int &count : count_stack)
                                        {
                                            count--;
                                        }

                                        if (count_stack[count_stack.size() - 1] == 0)
                                        {
                                            if (i != value.get_ListElements().size() - 1)
                                                cout << "), ";
                                            else
                                                cout << ")";

                                            count_stack.pop_back();
                                        }
                                        else
                                        {
                                            if (i != value.get_ListElements().size() - 1)
                                                cout << ", ";
                                        }
                                    }
                                    else
                                    {
                                        if (i != value.get_ListElements().size() - 1)
                                            cout << ", ";
                                    }
                                }
                            }
                            cout << ")";
                        }
                        else if (value.get_NodeType() == ObjectType::ENV || value.get_nodeValue() == "dummy")
                        {
                            cout << "dummy";
                        }
                        else if (value.get_NodeType() == ObjectType::LAMBDA)
                        {
                            cout << "[lambda closure: ";
                            cout << value.get_nodeValue() << ": ";
                            cout << value.get_CSIndex() << "]";
                        }
                        else
                        {
                            cout << value.get_nodeValue();
                        }
                    }

                    else if (identifier == "Isinteger")
                    {
                        CSENode value = stack.returnLastNode();
                        if (value.get_NodeType() == ObjectType::INTEGER)
                        {
                            stack.addNode(CSENode(ObjectType::BOOLEAN, "true"));
                        }
                        else
                        {
                            stack.addNode(CSENode(ObjectType::BOOLEAN, "false"));
                        }
                    }
                    else if (identifier == "Isstring")
                    {
                        CSENode value = stack.returnLastNode();
                        if (value.get_NodeType() == ObjectType::STRING)
                        {
                            stack.addNode(CSENode(ObjectType::BOOLEAN, "true"));
                        }
                        else
                        {
                            stack.addNode(CSENode(ObjectType::BOOLEAN, "false"));
                        }
                    }
                    else if (identifier == "Isempty")
                    {
                        CSENode value = stack.returnLastNode();
                        if (value.get_NodeType() == ObjectType::LIST)
                        {
                            if (value.get_ListElements().empty())
                            {
                                stack.addNode(CSENode(ObjectType::BOOLEAN, "true"));
                            }
                            else
                            {
                                stack.addNode(CSENode(ObjectType::BOOLEAN, "false"));
                            }
                        }
                        else
                        {
                            throw runtime_error("Invalid type for IsEmpty: " + value.get_nodeValue());
                        }
                    }
                    else if (identifier == "Istuple")
                    {
                        CSENode value = stack.returnLastNode();

                        if (value.get_NodeType() == ObjectType::LIST)
                        {
                            stack.addNode(CSENode(ObjectType::BOOLEAN, "true"));
                        }
                        else
                        {
                            stack.addNode(CSENode(ObjectType::BOOLEAN, "false"));
                        }
                    }
                    else if (identifier == "Order")
                    {
                        CSENode value = stack.returnLastNode();
                        if (value.get_NodeType() == ObjectType::LIST)
                        {
                            int count = 0;
                            int list_elem_skip = 0;

                            for (const auto &i : value.get_ListElements())
                            {
                                if (i.get_NodeType() == ObjectType::LIST && list_elem_skip == 0)
                                {
                                    list_elem_skip += stoi(i.get_nodeValue());
                                    count++;
                                }
                                else if (list_elem_skip == 0)
                                {
                                    count++;
                                }
                                else
                                {
                                    list_elem_skip--;
                                    continue;
                                }
                            }

                            stack.addNode(CSENode(ObjectType::INTEGER, to_string(count)));
                        }
                        else
                        {
                            throw runtime_error("Invalid type for Order: " + value.get_nodeValue());
                        }
                    }
                    else if (identifier == "Conc")
                    {
                        CSENode first_arg = stack.returnLastNode();
                        CSENode second_arg = stack.returnLastNode();
                        cse_machine.popLastNode();

                        if (first_arg.get_NodeType() == ObjectType::STRING &&
                            (second_arg.get_NodeType() == ObjectType::STRING ||
                             second_arg.get_NodeType() == ObjectType::INTEGER))
                        {
                            stack.addNode(
                                CSENode(ObjectType::STRING, first_arg.get_nodeValue() + second_arg.get_nodeValue()));
                        }
                        else
                        {
                            throw runtime_error("Invalid type for Conc: " + first_arg.get_nodeValue());
                        }
                    }
                    else if (identifier == "Stem")
                    {
                        CSENode arg = stack.returnLastNode();

                        if (arg.get_NodeType() == ObjectType::STRING)
                        {
                            stack.addNode(CSENode(ObjectType::STRING, arg.get_nodeValue().substr(0, 1)));
                        }
                        else
                        {
                            throw runtime_error("Invalid type for Stem: " + top_of_stack.get_nodeValue());
                        }
                    }

                    else if (identifier == "Stern")
                    {
                        CSENode arg = stack.returnLastNode();

                        if (arg.get_NodeType() == ObjectType::STRING)
                        {
                            stack.addNode(CSENode(ObjectType::STRING, arg.get_nodeValue().substr(1)));
                        }
                        else
                        {
                            throw runtime_error("Invalid type for Stern: " + top_of_stack.get_nodeValue());
                        }
                    }
                    else if (identifier == "Y*")
                    {
                        CSENode lambda = stack.returnLastNode();

                        if (lambda.get_NodeType() == ObjectType::LAMBDA)
                        {
                            if (lambda.get_IsSingleBoundVar())
                            {
                                stack.addNode(CSENode(ObjectType::EETA, lambda.get_nodeValue(), lambda.get_CSIndex(), lambda.get_ENV()));
                            }
                            else
                            {
                                stack.addNode(CSENode(ObjectType::EETA, lambda.get_CSIndex(), lambda.get_varList(), lambda.get_ENV()));
                            }
                        }
                        else
                        {
                            throw runtime_error("Invalid type for Y*: " + lambda.get_nodeValue());
                        }
                    }
                    else if (identifier == "ItoS")
                    {
                        CSENode arg = stack.returnLastNode();

                        if (arg.get_NodeType() == ObjectType::INTEGER)
                        {
                            stack.addNode(CSENode(ObjectType::STRING, arg.get_nodeValue()));
                        }
                        else
                        {
                            throw runtime_error("Invalid type for ItoS: " + arg.get_nodeValue());
                        }
                    }
                }
                else if (top_of_stack.get_NodeType() == ObjectType::EETA)
                {
                    stack.addNode(top_of_stack);

                    if (top_of_stack.get_IsSingleBoundVar())
                    {
                        stack.addNode(
                            CSENode(ObjectType::LAMBDA, top_of_stack.get_nodeValue(), top_of_stack.get_CSIndex(),
                                    top_of_stack.get_ENV()));
                    }
                    else
                    {
                        stack.addNode(
                            CSENode(ObjectType::LAMBDA, top_of_stack.get_CSIndex(), top_of_stack.get_varList(),
                                    top_of_stack.get_ENV()));
                    }

                    cse_machine.addNode(CSENode(ObjectType::GAMMA, ""));
                    cse_machine.addNode(CSENode(ObjectType::GAMMA, ""));
                }
                else if (top_of_stack.get_NodeType() == ObjectType::LIST)
                {
                    CSENode second_arg = stack.returnLastNode();

                    if (second_arg.get_NodeType() == ObjectType::INTEGER)
                    {
                        int index = stoi(second_arg.get_nodeValue());

                        int current_index = 0;
                        int list_element_pos = 0;
                        int list_elem_skip = 0;
                        bool is_list = false;

                        for (const auto &i : top_of_stack.get_ListElements())
                        {
                            if (i.get_NodeType() == ObjectType::LIST && list_elem_skip == 0)
                            {
                                list_elem_skip = stoi(i.get_nodeValue());
                                current_index++;

                                if (index == current_index)
                                {
                                    is_list = true;
                                    break;
                                }
                            }
                            else if (list_elem_skip == 0)
                            {
                                current_index++;

                                if (index == current_index)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                list_elem_skip--;
                            }
                            list_element_pos++;
                        }

                        vector<CSENode> listElements = vector<CSENode>();

                        if (is_list)
                        {
                            int length = stoi(top_of_stack.get_ListElements()[list_element_pos].get_nodeValue());

                            for (int i = 0; i < length; i++)
                            {
                                listElements.push_back(top_of_stack.get_ListElements()[list_element_pos + i + 1]);
                            }

                            stack.addNode(CSENode(ObjectType::LIST, listElements));
                        }
                        else
                        {
                            stack.addNode(top_of_stack.get_ListElements()[list_element_pos]);
                        }
                    }
                    else
                    {
                        throw runtime_error("Invalid type for Index: " + second_arg.get_nodeValue());
                    }
                }

                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::ENV)
            {
                vector<CSENode> env_nodes = {};

                CSENode st_node = stack.returnLastNode();

                while (st_node.get_NodeType() != ObjectType::ENV)
                {
                    env_nodes.push_back(st_node);
                    st_node = stack.returnLastNode();
                }

                // push back the stack from vector
                for (auto it = env_nodes.rbegin(); it != env_nodes.rend(); ++it)
                {
                    stack.addNode(*it);
                }

                env_stack.pop_back();

                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::OPERATOR)
            {
                string biop = top.get_nodeValue();

                CSENode val_1 = stack.returnLastNode();
                CSENode val_2 = stack.returnLastNode();

                if (biop == "+" || biop == "-" || biop == "/" || biop == "*")
                {
                    stack.addNode(CSENode(ObjectType::INTEGER, op(biop, val_1.get_nodeValue(), val_2.get_nodeValue())));
                }

                else if (biop == "neg")
                {
                    stack.addNode(val_2);
                    stack.addNode(CSENode(ObjectType::INTEGER, unop(biop, val_1.get_nodeValue())));
                }
                else if (biop == "not")
                {
                    stack.addNode(val_2);
                    stack.addNode(CSENode(ObjectType::BOOLEAN, unop(biop, val_1.get_nodeValue())));
                }
                else if (biop == "aug")
                {
                    if (val_1.get_NodeType() == ObjectType::LIST)
                    {
                        if (val_2.get_NodeType() == ObjectType::LIST)
                        {
                            vector<CSENode> elem_1 = val_1.get_ListElements();
                            vector<CSENode> elem_2 = val_2.get_ListElements();

                            elem_1.emplace_back(ObjectType::LIST, to_string(elem_2.size()));

                            for (auto &element : elem_2)
                            {
                                elem_1.push_back(element);
                            }

                            stack.addNode(CSENode(ObjectType::LIST, elem_1));
                        }
                        else if (val_2.get_NodeType() == ObjectType::INTEGER ||
                                 val_2.get_NodeType() == ObjectType::BOOLEAN ||
                                 val_2.get_NodeType() == ObjectType::STRING)
                        {
                            vector<CSENode> elem_1 = val_1.get_ListElements();

                            elem_1.emplace_back(val_2.get_NodeType(), val_2.get_nodeValue());
                            stack.addNode(CSENode(ObjectType::LIST, elem_1));
                        }
                        else
                        {
                            throw runtime_error("Invalid type for aug: " + val_2.get_nodeValue());
                        }
                    }
                }
                else
                {
                    // handle boolena values
                    stack.addNode(CSENode(ObjectType::BOOLEAN, booleanops(biop, val_1.get_nodeValue(), val_2.get_nodeValue())));
                }

                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::TAU)
            {
                vector<CSENode> new_elem;
                int tau_size = stoi(top.get_nodeValue());

                for (int i = 0; i < tau_size; i++)
                {
                    CSENode node = stack.returnLastNode();

                    if (node.get_NodeType() == ObjectType::LIST)
                    {
                        vector<CSENode> elem_1 = node.get_ListElements();
                        new_elem.emplace_back(ObjectType::LIST, to_string(elem_1.size()));

                        for (auto &element : elem_1)
                        {
                            new_elem.push_back(element);
                        }
                    }
                    else
                    {
                        new_elem.push_back(node);
                    }
                }

                stack.addNode(CSENode(ObjectType::LIST, new_elem));

                top = cse_machine.returnLastNode();
            }
            else if (top.get_NodeType() == ObjectType::BETA)
            {
                CSENode node = stack.returnLastNode();

                if (node.get_NodeType() == ObjectType::BOOLEAN)
                {
                    if (node.get_nodeValue() == "true")
                    {
                        cse_machine.popLastNode();
                        CSENode true_node = cse_machine.returnLastNode();

                        if (true_node.get_NodeType() == ObjectType::DELTA)
                        {
                            cse_machine.addNewCS(*controlStructures[stoi(true_node.get_nodeValue())]);
                        }
                        else
                        {
                            throw runtime_error("Invalid type for beta: " + true_node.get_nodeValue());
                        }
                    }
                    else
                    {
                        CSENode false_node = cse_machine.returnLastNode();
                        cse_machine.popLastNode();

                        if (false_node.get_NodeType() == ObjectType::DELTA)
                        {
                            cse_machine.addNewCS(*controlStructures[stoi(false_node.get_nodeValue())]);
                        }
                        else
                        {
                            throw runtime_error("Invalid type for beta: " + false_node.get_nodeValue());
                        }
                    }
                }
                else if (node.get_NodeType() == ObjectType::INTEGER)
                {
                    if (node.get_nodeValue() != "0")
                    {
                        cse_machine.popLastNode();
                        CSENode true_node = cse_machine.returnLastNode();

                        if (true_node.get_NodeType() == ObjectType::DELTA)
                        {
                            cse_machine.addNewCS(
                                *controlStructures[stoi(true_node.get_nodeValue())]);
                        }
                        else
                        {
                            throw runtime_error("Invalid type for beta: " + true_node.get_nodeValue());
                        }
                    }
                    else
                    {
                        CSENode false_node = cse_machine.returnLastNode();
                        cse_machine.popLastNode();

                        if (false_node.get_NodeType() == ObjectType::DELTA)
                        {
                            cse_machine.addNewCS(
                                *controlStructures[stoi(false_node.get_nodeValue())]);
                        }
                        else
                        {
                            throw runtime_error("Invalid type for beta: " + false_node.get_nodeValue());
                        }
                    }
                }
                else
                {
                    throw runtime_error("Invalid type for beta: " + node.get_nodeValue());
                }

                top = cse_machine.returnLastNode();
            }
        }
    }
};

bool isOperator(const string &label)
{
    vector<string> operators_ = {"+", "-", "/", "*", "aug", "neg", "not", "eq", "gr", "ge", "ls", "le", "ne", "or", "&"};

    auto it = find(operators_.begin(), operators_.end(), label);
    return it != operators_.end();
}

#endif // CSE_H