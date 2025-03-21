#include "UI.h"

UI::UI(HashTable* ht) : hashTable(ht) {
    for (int i = 0; i < 17; i++) {
        indexRects[i] = {10, float(110 + i * 40), 40, 30};  // Larger size: 50x50, starting at y=150, spacing 60
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
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int value = strlen(inputText) > 0 ? atoi(inputText) : -1;
        if (CheckCollisionPointRec(mousePoint, insertBtn) && value >= 0) {
            if (hashTable->find(value)) {
        resultMessage = "Duplicate: " + std::string(inputText) + " already exists";
        } else {
            hashTable->insert(value);
            resultMessage = "Inserted: " + std::string(inputText);
        }
            resultMessageTimer = 120;
            inputText[0] = '\0';
            inputActive = false;  // Turn off input after action
        }
        else if (CheckCollisionPointRec(mousePoint, removeBtn) && value >= 0) {
            bool success = hashTable->remove(value);
            resultMessage = success ? "Removed: " + std::string(inputText) : "Not found: " + std::string(inputText);
            resultMessageTimer = 120;
            inputText[0] = '\0';
            inputActive = false;  // Turn off input after action
        }
        else if (CheckCollisionPointRec(mousePoint, findBtn) && value >= 0) {
            bool found = hashTable->find(value);
            resultMessage = found ? "Found: " + std::string(inputText) : "Not found: " + std::string(inputText);
            resultMessageTimer = 120;
            inputText[0] = '\0';
            inputActive = false;  // Turn off input after action
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
        // Removed the else clause that set inputActive = true
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
    const int slotHeight = 40;  // Increased from 40 to match larger spacing
    const int slotWidth = 50;
    
    for (int i = 0; i < 17; i++) {
        // Draw index as a rectangle
        DrawRectangleRec(indexRects[i], LIGHTGRAY);
        DrawText(TextFormat("%d", i), indexRects[i].x + 10, indexRects[i].y + 5, 20, BLACK);  // Centered in rectangle
        
        Node* current = hashTable->getTable()[i];
        int xOffset = 70;  // Increased from 40 to account for wider index rectangle
        while (current) {
            DrawRectangle(xOffset, 110 + i * slotHeight, slotWidth, 30, LIGHTGRAY);  // Adjusted y-position to 150
            DrawText(TextFormat("%d", current->value), xOffset + 5, 115 + i * slotHeight, 20, BLACK);
            //if (current->next) {
                DrawLine(xOffset - 20, 125 + i * slotHeight, xOffset, 125 + i * slotHeight, BLACK);
            //}
            xOffset += 70;
            current = current->next;
        }
    }
}