#include "raylib.h"
#include "tinyfiledialogs.h"
#include<string>
#include<cstring>
#include<vector>
#include<utility>
#include<cstdlib>
#include<ctime>
#include<fstream>
#ifndef HASHTABLE_H
#define HASHTABLE_H


struct Node {
    int value;
    Node* next;
};

class HashTable {
private:
    static const int TABLE_SIZE = 19; // also change in UI.h
    std::vector<Node*> table;
    

public:
    int hashFunction(int value) const;
    HashTable();
    ~HashTable();
    void insert(int value);
    bool remove(int value);
    bool find(int value) const;
    void clear();
    void fillRandom(int count);
    const std::vector<Node*>& getTable() const { return table; }
};

#endif

#ifndef UI_H
#define UI_H

// #include "HashTable.h"


class UI {
public:
    static const int TABLE_SIZE = 19; //also change in HashTable.h
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
    std::vector<int> insertQueue;  // Queue for numbers from file
    Rectangle loadBtn = {560, 10, 100, 40};
    Rectangle insertBtn = {10, 10, 100, 40};
    Rectangle removeBtn = {120, 10, 100, 40};
    Rectangle findBtn = {230, 10, 100, 40};
    Rectangle clearBtn = {340, 10, 100, 40};
    Rectangle randomBtn = {450, 10, 100, 40};
    Rectangle indexRects[TABLE_SIZE];
    Rectangle inputBox = {800, 10, 100, 40};
    bool instantMode = false; 
    Rectangle instantBtn = {560, 55, 100, 40};  
    Vector2 inputLabelPos = {740, 20};

    void drawTable() const;
    void drawButtons() const;
    void drawInputBox() const;


};

#endif