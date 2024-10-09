#ifndef TREE_H
#define TREE_H

#include "TreeNode.h"

// Function to generate symbol table
void generateST(TreeNode *currentNode, TreeNode *parentNode);

// Tree structure
class Tree
{
private:
    static Tree *tree;           // Singleton instance of the tree
    TreeNode *astRoot = nullptr; // Root node of the abstract syntax tree (AST)
    TreeNode *stRoot = nullptr;  // Root node of the symbol table (ST)

    // Constructor and destructor
    Tree() {}
    ~Tree() {}

    Tree(const Tree &) = delete;
    Tree &operator=(const Tree &) = delete;

public:
    static Tree &getInstance() // Get the singleton instance of the tree
    {
        return *tree;
    }

    void setASTRoot(TreeNode *root) // Set the root node of the AST
    {
        astRoot = root;
    }

    TreeNode *getASTRoot() // Get the root node of the AST
    {
        return astRoot;
    }

    void setSTRoot(TreeNode *root) // Set the root node of the symbol table
    {
        stRoot = root;
    }

    TreeNode *getSTRoot() // Get the root node of the symbol table
    {
        return stRoot;
    }

    static void releaseASTMemory() // Release the memory of the AST
    {
        if (tree->astRoot != nullptr)
        {
            if (tree->stRoot == nullptr)
            {
                tree->stRoot = tree->astRoot;
            }
            tree->astRoot = nullptr;
        }
    }

    static void releaseSTMemory() // Release the memory of the symbol table
    {
        TreeNode::releaseNodeMemory(tree->stRoot);
    }

    // Generate symbol table from the AST
    static void generate()
    {
        releaseASTMemory();
        generateST(tree->stRoot, nullptr);
    }
};

#endif // TREE_H
