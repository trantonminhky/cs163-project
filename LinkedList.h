#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <vector>
#include <stack>
#include <utility>
#include "tinyfiledialogs.h"
#include <locale>
#include <codecvt>
#include <string>

struct NodeL {
    int value;
    NodeL* next;
    float x, y;           // Current position for animation
    float targetX, targetY; // Target position for animation
    NodeL(int val);
};

class LinkedList {
private:
    NodeL* head;
    std::stack<std::pair<NodeL*, std::pair<bool, int>>> history; // <list state, <wasInsert, value>>
    std::stack<std::pair<NodeL*, std::pair<bool, int>>> redoStack;

    NodeL* insert(NodeL* node, int value, std::vector<NodeL*>& path);
    NodeL* deleteNode(NodeL* node, int value);
    void deleteList(NodeL* node);
    NodeL* deepCopy(NodeL* node);
    void calculatePositions(NodeL* node, int startX, int startY, int xOffset);
    void drawNode(NodeL* node, const std::vector<NodeL*>& highlightPath);

public:
    LinkedList();
    ~LinkedList();
    void insert(int value);
    void deleteNode(int value);
    void search(int value, std::vector<NodeL*>& searchPath);
    void clear();
    NodeL* undo(std::vector<NodeL*>& affectedPath);
    NodeL* redo(std::vector<NodeL*>& affectedPath);
    void clearHistory();
    void generateRandom(int count, int minValue, int maxValue);
    void updateAnimation(float deltaTime);
    void draw(const std::vector<NodeL*>& highlightPath);
    void LoadFromFile(std::string& searchResult);
    bool instantMode; // Added for instant execution toggle
    Rectangle instantBtn; // Added for instant mode button
};

#endif