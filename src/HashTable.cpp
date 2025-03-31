#include "HashTable.h"


HashTable::HashTable() {
    table.resize(TABLE_SIZE, nullptr);
}

HashTable::~HashTable() {
    clear();
}

int HashTable::hashFunction(int value) const {
    return value % TABLE_SIZE;
}

void HashTable::insert(int value) {
    if (find(value)) {
        return;
    }
    
    int index = hashFunction(value);
    Node* newNode = new Node{value, nullptr};
    
    if (!table[index]) {
        table[index] = newNode;
    } else {
        Node* current = table[index];
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

bool HashTable::remove(int value) {
    int index = hashFunction(value);
    Node* current = table[index];
    Node* prev = nullptr;

    while (current) {
        if (current->value == value) {
            if (prev) {
                prev->next = current->next;
            } else {
                table[index] = current->next;
            }
            delete current;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

bool HashTable::find(int value) const {
    int index = hashFunction(value);
    Node* current = table[index];
    
    while (current) {
        if (current->value == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void HashTable::clear() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* current = table[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
        table[i] = nullptr;
    }
}

void HashTable::fillRandom(int count) {
    clear();
    srand(time(nullptr));
    for (int i = 0; i < count; i++) {
        insert(rand() % 100); // Random numbers 0-99
    }
}




UI::UI(HashTable* ht) : hashTable(ht) {
    for (int i = 0; i < UI::TABLE_SIZE; i++) {
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
                animTimer = 30;
                break;
            case AnimationState::EXISTING_NODES:
                if (animStep < (int)existingValues.size()) {
                    animStep++;
                    animTimer = 30;
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
if (animState == AnimationState::NONE && !insertQueue.empty()) {
    pendingInsertValue = insertQueue.back();
    insertQueue.pop_back();
    resultMessage = "Inserting: " + std::to_string(pendingInsertValue);
    animState = AnimationState::INDEX;
    animIndex = hashTable->hashFunction(pendingInsertValue);
    existingValues.clear();
    Node* current = hashTable->getTable()[animIndex];
    while (current) {
        existingValues.push_back(current->value);
        current = current->next;
    }
    animStep = 0;
    animTimer = 30; 
    resultMessageTimer = 120; 
}

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int value = strlen(inputText) > 0 ? atoi(inputText) : -1;
        if (CheckCollisionPointRec(mousePoint, insertBtn) && value >= 0) {
        if (hashTable->find(value)) {
            resultMessage = "Duplicate: " + std::string(inputText) + " already exists";
            highlightedValue = value;
            highlightTimer = 120;
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
        animTimer = 30;
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
        animTimer = 30;
        } else {
        resultMessage = "Not found: " + std::string(inputText);
        }
            resultMessageTimer = 120;
            inputText[0] = '\0';
            inputActive = false;
        }
        else if (CheckCollisionPointRec(mousePoint, findBtn) && value >= 0) {
        if (hashTable->find(value)) {
        resultMessage = "Found: " + std::string(inputText);
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
        animTimer = 30;
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
            insertQueue.clear();         
            pendingInsertValue = -1;       
            animState = AnimationState::NONE;  
            animIndex = -1;
            existingValues.clear();
            animStep = 0;
            animTimer = 0;
        }
        else if (CheckCollisionPointRec(mousePoint, randomBtn)) {
            hashTable->fillRandom(20);
            resultMessage = "Generated random table";
            resultMessageTimer = 120;
            inputActive = false;  // Turn off input after action
        }
        else if (CheckCollisionPointRec(mousePoint, loadBtn)) {
            static const char* filterPaterns[] = {"*.txt", nullptr};
            const char* filePath = tinyfd_openFileDialog(
                "Select a Text File",       // Title
                "",                         // Default path (empty = current directory)
                1,                          // Number of filter patterns
                filterPaterns,   // Filter patterns
                "Text files",               // Filter description
                0                           // 0 = single file selection
            );
            if (filePath) {  // If a file was selected
                std::ifstream file(filePath);
                if (file.is_open()) {
                    insertQueue.clear();
                    int num;
                    if (instantMode) {
                        while (file >> num) {
                            if (num >= 0 && num <= 999) {
                                hashTable->insert(num);
                            }
                        }
                        resultMessage = "Numbers loaded instantly from " + std::string(filePath);
                    } else {
                        while (file >> num) {
                            if (num >= 0 && num <= 999) {
                                insertQueue.push_back(num);
                            }
                        }
                        resultMessage = "Loading numbers from " + std::string(filePath);
                    }
                    file.close();
                } else {
                    resultMessage = "Failed to open selected file";
                }
            } else {
                resultMessage = "No file selected";
            }
            resultMessageTimer = 120;
            inputActive = false;
}
        else if (CheckCollisionPointRec(mousePoint, instantBtn)) {
            instantMode = !instantMode;  // Toggle mode
            resultMessage = instantMode ? "Instant Mode ON" : "Instant Mode OFF";
            resultMessageTimer = 120;
            inputActive = false;
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
    DrawRectangleRec(loadBtn, GRAY);
    DrawRectangleRec(instantBtn, instantMode ? PINK : LIME);
    
    DrawText("Insert", insertBtn.x + 20, insertBtn.y + 10, 20, WHITE);
    DrawText("Remove", removeBtn.x + 15, removeBtn.y + 10, 20, WHITE);
    DrawText("Find", findBtn.x + 30, findBtn.y + 10, 20, WHITE);
    DrawText("Clear", clearBtn.x + 25, clearBtn.y + 10, 20, WHITE);
    DrawText("Random", randomBtn.x + 15, randomBtn.y + 10, 20, WHITE);
    DrawText("Load", loadBtn.x + 25, loadBtn.y + 10, 20, WHITE);
    DrawText(instantMode ? "Instant" : "  Slow", instantBtn.x + 15, instantBtn.y + 10, 20, BLACK);
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
    
    for (int i = 0; i < UI::TABLE_SIZE; i++) {
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