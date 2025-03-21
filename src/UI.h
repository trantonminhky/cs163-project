#ifndef UI_H
#define UI_H

#include "HashTable.h"
#include "../include/raylib.h"
#include<string>
#include<cstring>

class UI {
private:
    static const int TABLE_SIZE = 17;
    HashTable* hashTable;
    char inputText[4] = "\0";  // For number input (max 3 digits + null)
    bool inputActive = false;
    int resultMessageTimer = 0;
    std::string resultMessage;
    
    Rectangle insertBtn = {10, 10, 100, 40};
    Rectangle removeBtn = {120, 10, 100, 40};
    Rectangle findBtn = {230, 10, 100, 40};
    Rectangle clearBtn = {340, 10, 100, 40};
    Rectangle randomBtn = {450, 10, 100, 40};
    Rectangle indexRects[17];
    Rectangle inputBox = {700, 10, 100, 40};
    Vector2 inputLabelPos = {640, 20};

    void drawTable() const;
    void drawButtons() const;
    void drawInputBox() const;

public:
    UI(HashTable* ht);
    void update();
    void draw();
};

#endif