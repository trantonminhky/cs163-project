#include "UI.h"



UI::UI(HashTable* ht) : hashTable(ht) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        indexRects[i] = {10, float(110 + i * 40), 40, 30};  
    }
}

void UI::update() {
    Vector2 mousePoint = GetMousePosition();
    
    // Check if input box is clicked to toggle inputActive
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePoint, inputBox)) {
        if (!inputActive) {  // Only clear when activating, not deactivating
            for (int i=0;i<4;++i) inputText[i]='\0';  // Clear the input buffer
        }
        inputActive = !inputActive;  // Toggle input active state
    }
    
    if (inputActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 48 && key <= 57) && strlen(inputText) < 3) {
                inputText[strlen(inputText)] = (char)key;
                inputText[strlen(inputText) + 1] = '\0';
            }
            key = GetCharPressed();
        }
    }

if (animState != AnimationState::NONE) {
    if (animTimer > 0) {
        animTimer--;
    } else {
        switch (animState) {
            case AnimationState::INDEX:
                animState = AnimationState::EXISTING_NODES;
                animTimer = 15;
                break;
            case AnimationState::EXISTING_NODES:
                if (animStep < (int)existingValues.size()) {
                    animStep++;
                    animTimer = 15;
                } else {
                    // Handle completion based on operation
                    if (pendingInsertValue != -1) {  // Insert
                        hashTable->insert(pendingInsertValue);
                        pendingInsertValue = -1;
                    } else if (pendingRemoveValue != -1) {  // Remove
                        hashTable->remove(pendingRemoveValue);
                        pendingRemoveValue = -1;
                    }  // Find doesn’t need a final action
                    animState = AnimationState::NONE;
                    animIndex = -1;
                    existingValues.clear();
                    animStep = 0;
                }
                break;
            case AnimationState::NEW_NODE:  // Used for insert only
                if (pendingInsertValue != -1) {
                    hashTable->insert(pendingInsertValue);
                    pendingInsertValue = -1;
                }
                animState = AnimationState::NONE;
                animIndex = -1;
                existingValues.clear();
                animStep = 0;
                break;
            default:
                break;
        }
    }
}

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int value = strlen(inputText) > 0 ? atoi(inputText) : -1;
        if (CheckCollisionPointRec(mousePoint, insertBtn) && value >= 0) {
        if (hashTable->find(value)) {
            resultMessage = "Duplicate: " + std::string(inputText) + " already exists";
            highlightedValue = value;
            highlightTimer = 60;
        } else {
        // Don’t insert yet, store the value and start animation
        pendingInsertValue = value;
        resultMessage = "Inserting: " + std::string(inputText);
        animState = AnimationState::INDEX;
        animIndex = hashTable->hashFunction(value);
        existingValues.clear();
        Node* current = hashTable->getTable()[animIndex];
        while (current) {
            existingValues.push_back(current->value);
            current = current->next;
        }
        animStep = 0;
        animTimer = 15;
        }
        resultMessageTimer = 120;
        inputText[0] = '\0';
        inputActive = false;
        }
        else if (CheckCollisionPointRec(mousePoint, removeBtn) && value >= 0) {
        if (hashTable->find(value)) {
        pendingRemoveValue = value;  // Store value to remove later
        resultMessage = "Removing: " + std::string(inputText);
        animState = AnimationState::INDEX;
        animIndex = hashTable->hashFunction(value);
        existingValues.clear();
        Node* current = hashTable->getTable()[animIndex];
        while (current && current->value != value) {  // Stop at the value to remove
            existingValues.push_back(current->value);
            current = current->next;
        }
        if (current) existingValues.push_back(current->value);  // Include the node to remove
        animStep = 0;
        animTimer = 15;
        } else {
        resultMessage = "Not found: " + std::string(inputText);
        }
            resultMessageTimer = 120;
            inputText[0] = '\0';
            inputActive = false;
        }
else if (CheckCollisionPointRec(mousePoint, findBtn) && value >= 0) {
    if (hashTable->find(value)) {
        resultMessage = "Finding: " + std::string(inputText);
        animState = AnimationState::INDEX;
        animIndex = hashTable->hashFunction(value);
        existingValues.clear();
        Node* current = hashTable->getTable()[animIndex];
        while (current && current->value != value) {  // Stop at the value to find
            existingValues.push_back(current->value);
            current = current->next;
        }
        if (current) existingValues.push_back(current->value);  // Include the found node
        animStep = 0;
        animTimer = 15;
        } else {
            resultMessage = "Not found: " + std::string(inputText);
        }
        resultMessageTimer = 120;
        inputText[0] = '\0';
        inputActive = false;
        }
        else if (CheckCollisionPointRec(mousePoint, clearBtn)) {
            hashTable->clear();
            resultMessage = "Table cleared";
            resultMessageTimer = 120;
            inputActive = false;  // Turn off input after action
        }
        else if (CheckCollisionPointRec(mousePoint, randomBtn)) {
            hashTable->fillRandom(20);
            resultMessage = "Generated random table";
            resultMessageTimer = 120;
            inputActive = false;  // Turn off input after action
        }
    }
    
    if (resultMessageTimer > 0) resultMessageTimer--;
}

void UI::draw() {
    drawButtons();
    drawInputBox();
    drawTable();
    
    if (resultMessageTimer > 0) {
        DrawText(resultMessage.c_str(), 10, 60, 20, DARKGRAY);
    }
}

void UI::drawButtons() const {
    DrawRectangleRec(insertBtn, GREEN);
    DrawRectangleRec(removeBtn, ORANGE);
    DrawRectangleRec(findBtn, BLUE);
    DrawRectangleRec(clearBtn, RED);
    DrawRectangleRec(randomBtn, PURPLE);
    
    DrawText("Insert", insertBtn.x + 20, insertBtn.y + 10, 20, WHITE);
    DrawText("Remove", removeBtn.x + 15, removeBtn.y + 10, 20, WHITE);
    DrawText("Find", findBtn.x + 30, findBtn.y + 10, 20, WHITE);
    DrawText("Clear", clearBtn.x + 25, clearBtn.y + 10, 20, WHITE);
    DrawText("Random", randomBtn.x + 15, randomBtn.y + 10, 20, WHITE);
}

void UI::drawInputBox() const {
    
    DrawRectangleRec(inputBox, inputActive ? YELLOW : WHITE);
    DrawRectangleLinesEx(inputBox, 2, DARKGRAY);
    DrawText(inputText, inputBox.x + 10, inputBox.y + 10, 20, BLACK);
    DrawText("Input", inputLabelPos.x, inputLabelPos.y, 20, DARKGRAY);
}

void UI::drawTable() const {
    const int slotHeight = 40; 
    const int slotWidth = 50;
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        // Draw index as a rectangle
        DrawRectangleRec(indexRects[i], BLACK);
        if (animState == AnimationState::INDEX && i == animIndex) {
            DrawRectangleLinesEx(indexRects[i], 3, YELLOW);
        }
        DrawText(TextFormat("%d", i), indexRects[i].x + 10, indexRects[i].y + 5, indexRects[i].height - 10, WHITE); 

        
        Node* current = hashTable->getTable()[i];
        int xOffset = 70; 
        int nodeIndex = 0;
        while (current) {
            Rectangle valueRect = {float(xOffset), float(110 + i * slotHeight), slotWidth, 30};
            DrawRectangleRec(valueRect, LIGHTGRAY);
            // Glow existing nodes in sequence
            if (animState == AnimationState::EXISTING_NODES && i == animIndex && 
                nodeIndex < animStep && nodeIndex < (int)existingValues.size() && 
                current->value == existingValues[nodeIndex]) {
                DrawRectangleLinesEx(valueRect, 3, YELLOW);
            }
            // Glow new node
            if (animState == AnimationState::NEW_NODE && i == animIndex && !current->next && pendingInsertValue!=-1) {
                DrawRectangleLinesEx(valueRect, 3, YELLOW);
            }
            DrawText(TextFormat("%d", current->value), xOffset + 5, 115 + i * slotHeight, 20, BLACK);
                DrawLine(xOffset - 20, 125 + i * slotHeight, xOffset, 125 + i * slotHeight, BLACK);
            xOffset += 70;
            current = current->next;
            nodeIndex++;
        }
            if (animState == AnimationState::NEW_NODE && i == animIndex && pendingInsertValue != -1) {
            Rectangle newRect = {float(xOffset), float(110 + i * slotHeight), slotWidth, 30};
            DrawRectangleRec(newRect, LIGHTGRAY);
            DrawRectangleLinesEx(newRect, 3, YELLOW);
            DrawText(TextFormat("%d", pendingInsertValue), xOffset + 5, 115 + i * slotHeight, 20, BLACK);
        }
    }
}