#ifndef AVL_H
#define AVL_H

#include <vector>
#include <stack>
#include <utility>
#include "tinyfiledialogs.h"
#include <locale>
#include <codecvt>
#include <string>

struct Node {
    int key;
    int height;
    Node* left;
    Node* right;
    float x, y;           // Current position for animation
    float targetX, targetY; // Target position for animation
    bool isDying;         // Flag for deletion animation (unused now, kept for compatibility)
    Node(int value);
};

class AVLTree {
private:
    Node* root;
    std::stack<std::pair<Node*, std::pair<bool, int>>> history; // <tree state, <wasInsert, value>>
    std::stack<std::pair<Node*, std::pair<bool, int>>> redoStack;

    Node* insert(Node* node, int key, std::vector<Node*>& path);
    Node* deleteNode(Node* node, int key);
    Node* rightRotate(Node* y);
    Node* leftRotate(Node* x);
    Node* findMin(Node* node);
    void deleteTree(Node* node);
    int getHeight(Node* node);
    int getBalance(Node* node);
    void updateHeight(Node* node);
    void calculatePositions(Node* node, int x, int y, int xOffset, int depth);
    void drawNode(Node* node, const std::vector<Node*>& highlightPath);
    Node* deepCopy(Node* node);

public:
    AVLTree();
    ~AVLTree();
    void insert(int key);
    void deleteNode(int key);
    void search(int key, std::vector<Node*>& searchPath);
    void clear();
    Node* undo(std::vector<Node*>& affectedPath);
    Node* redo(std::vector<Node*>& affectedPath);
    void clearHistory();
    void generateRandom(int count, int minValue, int maxValue);
    void updateAnimation(float deltaTime);
    void draw(const std::vector<Node*>& highlightPath);
    void LoadFromFile(std::string& searchResult); // Already included
};

#endif