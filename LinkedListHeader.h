#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <vector>
#include <stack>
#include <utility>

struct Node {
    int value;
    Node* next;
    float x, y;           // Current position for animation
    float targetX, targetY; // Target position for animation
    Node(int val);
};

class LinkedList {
private:
    Node* head;
    std::stack<std::pair<Node*, std::pair<bool, int>>> history; // <list state, <wasInsert, value>>
    std::stack<std::pair<Node*, std::pair<bool, int>>> redoStack;

    Node* insert(Node* node, int value, std::vector<Node*>& path);
    Node* deleteNode(Node* node, int value, std::vector<Node*>& affectedPath);
    void deleteList(Node* node);
    Node* deepCopy(Node* node);
    void calculatePositions(Node* node, int startX, int startY);
    void drawNode(Node* node, const std::vector<Node*>& highlightPath);

public:
    LinkedList();
    ~LinkedList();
    void insert(int value);
    void deleteNode(int value);
    void search(int value, std::vector<Node*>& searchPath);
    void clear();
    Node* undo(std::vector<Node*>& affectedPath);
    Node* redo(std::vector<Node*>& affectedPath);
    void clearHistory();
    void generateRandom(int count, int minValue, int maxValue);
    void updateAnimation(float deltaTime);
    void draw(const std::vector<Node*>& highlightPath);
};

#endif