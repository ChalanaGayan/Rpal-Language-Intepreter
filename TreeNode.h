#ifndef TREENODE_H
#define TREENODE_H

#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>

using namespace std;

// TreeNode Structure representing a node in the tree
class TreeNode
{
private:
    string label;                // Label of the node
    vector<TreeNode *> children; // Children nodes of the current node
    string value;                // Value associated with the node

public:
    // Constructor initializing the label
    explicit TreeNode(string lbl) : label(move(lbl))
    {
        children = vector<TreeNode *>(); // Initialize children vector
    }

    // Add a child node to the current node
    void addChild(TreeNode *child)
    {
        children.push_back(child);
    }

    // Reverse the order of children
    void reverseChildren()
    {
        reverse(children.begin(), children.end());
    }

    // Remove a child node from the current node
    void removeChild(int index = 0, bool deleteNode = false)
    {
        if (index < 0 || index >= children.size())
        {
            throw out_of_range("Index out of range");
        }

        if (deleteNode)
        {
            releaseNodeMemory(children[index]);
        }

        children.erase(children.begin() + index);
    }

    // Get the number of children
    int getNumChildren()
    {
        return children.size();
    }

    // Get the label of the node
    string getLabel()
    {
        return label;
    }

    // Get a reference to the children vector
    vector<TreeNode *> &getChildren()
    {
        return children;
    }

    // Get the value of the node
    virtual string getValue()
    {
        return value;
    }

    // Set the value of the node
    void setValue(string v)
    {
        value = move(v);
    }

    // Release memory of the node and its children recursively
    static void releaseNodeMemory(TreeNode *node)
    {
        if (node == nullptr)
            return;

        for (TreeNode *child : node->getChildren())
        {
            releaseNodeMemory(child);
        }

        delete node;
    }
};

// InternalNode representing an internal node in the tree
class InternalNode : public TreeNode
{
public:
    // Constructor for internal node with a label and optional value
    InternalNode(const string &lbl, const string &v = " ") : TreeNode(lbl)
    {
        setValue(" ");
    }
};

// LeafNode representing a leaf node in the tree
class LeafNode : public TreeNode
{
public:
    // Constructor for leaf node with a label and value
    LeafNode(const string &lbl, const string &v) : TreeNode(lbl)
    {
        setValue(v);
    }

    // Prevent adding children to leaf nodes
    void addChild(TreeNode *child) = delete;
};

#endif // TREENODE_H
