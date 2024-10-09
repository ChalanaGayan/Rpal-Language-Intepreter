#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include "Parser.h"
#include "Token.h"
#include "CSEMachine.h"
#include "BOP/binaryOP.h"
#include "Tree.h"

#include "TreeNode.h"

using namespace std;
namespace fs = std::filesystem; // Namespace alias for filesystem

// Map the enum values
unordered_map<tokenType, string> token_typeNames = {
    {tokenType::IDENTIFIER, "IDENTIFIER"},
    {tokenType::INTEGER, "INTEGER"},
    {tokenType::STRING, "STRING"},
    {tokenType::OPERATOR, "OPERATOR"},
    {tokenType::DELIMITER, "DELIMITER"},
    {tokenType::KEYWORD, "KEYWORD"},
    {tokenType::END_OF_FILE, "END_OF_FILE"}};

// Get token name
string gettoken_typeName(tokenType type)
{
    if (token_typeNames.count(type) > 0)
    {
        return token_typeNames[type];
    }
    return "UNKNOWN";
}

// Check if Graphviz is installed
bool isGraphvizInstalled()
{
#ifdef _WIN32
    const char *redirectCommand = "2>nul";
#else
    const char *redirectCommand = "2>/dev/null";
#endif
    string command = "dot -V ";
    command += redirectCommand;
    int exitCode = system(command.c_str());
    return (exitCode == 0);
}

// Display a warning message if Graphviz is not installed
void displayGraphvizWarning()
{
    cout << " Please Download graphviz tool from: https://graphviz.org/download/" << endl;
}

// Helper function to generate dot file
int generateDotFileHelper(TreeNode *node, ofstream &file, int parent = -1, int nodeCount = 0)
{
    int currentNode = nodeCount;
    int nextNodeCount = nodeCount + 1;

    // Define new colors for visualization
    string labelColor = "darkorange";                                                                 // Change label color to orange
    string valueColor = "darkred";                                                                    // Change value color to dark red
    string fillColor = (node->getValue() == " " || node->getValue().empty()) ? "#DDDDDD" : "#EEEEEE"; // Change fill color to light gray or silver

    // Escape label characters if necessary
    string escapedLabel = node->getLabel();
    size_t pos1 = escapedLabel.find('&');
    while (pos1 != string::npos)
    {
        escapedLabel.replace(pos1, 1, "&amp;");
        pos1 = escapedLabel.find('&', pos1 + 5);
    }

    size_t pos = escapedLabel.find('>');
    while (pos != string::npos)
    {
        escapedLabel.replace(pos, 1, "&gt;");
        pos = escapedLabel.find('>', pos + 4);
    }

    // Prepare label and value strings for the dot file
    string labelStr = (escapedLabel.empty()) ? "&nbsp;" : escapedLabel;
    string valueStr = (node->getValue().empty()) ? "&nbsp;" : node->getValue();

    size_t pos2 = valueStr.find('\n');
    while (pos2 != string::npos)
    {
        valueStr.replace(pos2, 1, "\\n");
        pos2 = valueStr.find('\n', pos2 + 2);
    }

    // Write node details to the dot file
    file << "    node" << currentNode << " [label=<";
    file << "<font color=\"" << labelColor << "\">" << labelStr << "</font><br/>";
    file << "<font color=\"" << valueColor << "\">" << valueStr << "</font>";
    file << ">, style=filled, fillcolor=\"" << fillColor << "\"];\n";

    // Create an edge between the current node and its parent
    if (currentNode != 0 && parent != -1)
    {
        file << "    node" << parent << " -> node" << currentNode << ";\n";
    }

    // Recursively generate dot file contents for each child node
    for (TreeNode *child : node->getChildren())
    {
        nextNodeCount = generateDotFileHelper(child, file, currentNode, nextNodeCount);
    }

    return nextNodeCount;
}

// Generate dot file for the AST
void generateDotFile(TreeNode *root, const string &filename)
{
    string f_name = fs::absolute(fs::path("vizualise") / filename).string();
    ofstream file(f_name);
    if (file.is_open())
    {
        file << "digraph Tree {\n";
        generateDotFileHelper(root, file, 0);
        file << "}\n";
        file.close();
    }
}
// Helper function to print AST recursively
void printAST(TreeNode *node, int depth = 0)
{
    if (node == nullptr)
        return;

    // Print the current node's label and value
    cout << string(depth * 4, ' ') << node->getLabel() << ": " << node->getValue() << endl;

    // Recursively print child nodes
    for (TreeNode *child : node->getChildren())
    {
        printAST(child, depth + 1);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2 || string(argv[1]) == "-ast") // check user want to visualize AST or not
    {
        cout << "ERROR: Usage: .\\rpal20 input_file [-ast=VALUE]\n"
             << endl;
        return 1;
    }

    string filename = argv[1];
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Unable to open file: " << filename << endl;
        return 1;
    }

    string input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    string visualizeArg;
    bool visualizeAst = false;
    bool visualizeSt = false;

    for (int i = 2; i < argc; ++i)
    {
        string arg(argv[i]);
        if (arg == "-ast")
        {
            visualizeAst = true;
        }
    }

    if (!isGraphvizInstalled() && (visualizeAst || visualizeSt))
    {
        displayGraphvizWarning();
        visualizeAst = false;
        visualizeSt = false;
    }

    if (isGraphvizInstalled() && (visualizeSt || visualizeAst))
    {
        if (!fs::exists("vizualise"))
        {
            if (!fs::create_directory("vizualise"))
            {
                cout << "Failed to create vizualise directory!" << endl;
                return 1;
            }
        }
    }

    CustomLexer lexer(input);
    TokenController &tokenController = TokenController::getInstance();
    tokenController.setLexer(lexer);

    Parser::parse();
    TokenController::destroyInstance();
    TreeNode *root = Tree::getInstance().getASTRoot();

    if (visualizeAst)
    {
        generateDotFile(root, "ast.dot");
        string dotFilePath = fs::absolute(fs::path("vizualise") / "ast.dot").string();
        string outputFilePath = fs::absolute(fs::path("vizualise") / "ast.png").string();
        string command = "dot -Tpng -Gdpi=150 \"" + dotFilePath + "\" -o \"" + outputFilePath + "\"";
        system(command.c_str());

        cout << "Abstract Syntax Tree:" << endl;
        printAST(root);
        cout << "Also you can find the ast.png in the vizualise directory." << endl;
        exit(0);
    }

    Tree::generate();
    TreeNode *st_root = Tree::getInstance().getSTRoot();

    CSE cse = CSE();
    cse.createCS(Tree::getInstance().getSTRoot());
    cout << "Output of the above program is:" << endl;
    cse.evaluate();

    return 0;
}
