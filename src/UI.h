#ifndef UI_H
#define UI_H

#include "HashTable.h"
#include "../include/raylib.h"
#include<string>
#include<cstring>

class UI {
public:
    static const int TABLE_SIZE = 7; //also change in HashTable.h
    UI(HashTable* ht);
    void update();
    void draw();
private:
    HashTable* hashTable;
    char inputText[4] = "\0";  // For number input (max 3 digits + null)
    bool inputActive = false;
    int resultMessageTimer = 0;
    std::string resultMessage;
    int highlightedValue = -1;  
    int highlightTimer = 0;    
    enum class AnimationState { NONE, INDEX, EXISTING_NODES, NEW_NODE };
    AnimationState animState = AnimationState::NONE;
    int animIndex = -1;        // Index being animated
    std::vector<int> existingValues;  // Values of existing nodes to animate
    int animStep = 0;          // Current step in animation (node index)
    int animTimer = 0;         // Timer for animation delays
    int pendingInsertValue = -1;
    int pendingRemoveValue = -1;
    Rectangle insertBtn = {10, 10, 100, 40};
    Rectangle removeBtn = {120, 10, 100, 40};
    Rectangle findBtn = {230, 10, 100, 40};
    Rectangle clearBtn = {340, 10, 100, 40};
    Rectangle randomBtn = {450, 10, 100, 40};
    Rectangle indexRects[TABLE_SIZE];
    Rectangle inputBox = {700, 10, 100, 40};
    Vector2 inputLabelPos = {640, 20};

    void drawTable() const;
    void drawButtons() const;
    void drawInputBox() const;


};

#endif