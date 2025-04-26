#include "raylib.h"
#include "LinkedList.h"
#include "Common.h"
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <random>
#include <stdio.h>

NodeL::NodeL(int val) : value(val), next(nullptr), x(0), y(0), targetX(0), targetY(0) {}

LinkedList::LinkedList() : head(nullptr), instantMode(false) {
    const int screenHeight = 1000;
    instantBtn = { 900, screenHeight - 120, 100, 40 }; // Initialize instant button
}

LinkedList::~LinkedList() {
    deleteList(head);
    clearHistory();
}

void LinkedList::deleteList(NodeL* node) {
    while (node) {
        NodeL* temp = node;
        node = node->next;
        delete temp;
    }
}

NodeL* LinkedList::deepCopy(NodeL* node) {
    if (!node) return nullptr;
    NodeL* newNode = new NodeL(node->value);
    newNode->x = node->x;
    newNode->y = node->y;
    newNode->targetX = node->targetX;
    newNode->targetY = node->targetY;
    newNode->next = deepCopy(node->next);
    return newNode;
}

NodeL* LinkedList::insert(NodeL* node, int value, std::vector<NodeL*>& path) {
    if (!node) {
        NodeL* newNode = new NodeL(value);
        path.push_back(newNode);
        return newNode;
    }

    path.push_back(node);
    node->next = insert(node->next, value, path);
    return node;
}

void LinkedList::insert(int value) {
    NodeL* listCopy = deepCopy(head);
    history.push({ listCopy, {true, value} });
    while (!redoStack.empty()) {
        deleteList(redoStack.top().first);
        redoStack.pop();
    }

    std::vector<NodeL*> path;
    head = insert(head, value, path);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
}

NodeL* LinkedList::deleteNode(NodeL* node, int value) {
    if (!node) return nullptr;

    if (node->value == value) {
        NodeL* temp = node->next;
        delete node;
        return temp;
    }

    node->next = deleteNode(node->next, value);
    return node;
}

void LinkedList::deleteNode(int value) {
    if (!head) return;

    NodeL* listCopy = deepCopy(head);
    history.push({ listCopy, {false, value} });
    while (!redoStack.empty()) {
        deleteList(redoStack.top().first);
        redoStack.pop();
    }

    head = deleteNode(head, value);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
}

void LinkedList::search(int value, std::vector<NodeL*>& searchPath) {
    searchPath.clear();
    NodeL* current = head;
    while (current) {
        searchPath.push_back(current);
        if (current->value == value) break;
        current = current->next;
    }
}

void LinkedList::clear() {
    deleteList(head);
    head = nullptr;
    clearHistory();
}

NodeL* LinkedList::undo(std::vector<NodeL*>& affectedPath) {
    if (history.empty()) return nullptr;
    std::pair<NodeL*, std::pair<bool, int>> operation = history.top();
    NodeL* previousState = operation.first;
    bool wasInsert = operation.second.first;
    int value = operation.second.second;
    history.pop();

    NodeL* currentState = deepCopy(head);
    redoStack.push({ currentState, {wasInsert, value} });

    deleteList(head);
    head = previousState;

    affectedPath.clear();
    search(value, affectedPath);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
    return affectedPath.empty() ? nullptr : affectedPath.back();
}

NodeL* LinkedList::redo(std::vector<NodeL*>& affectedPath) {
    if (redoStack.empty()) return nullptr;
    std::pair<NodeL*, std::pair<bool, int>> operation = redoStack.top();
    NodeL* redoState = operation.first;
    bool wasInsert = operation.second.first;
    int value = operation.second.second;
    redoStack.pop();

    NodeL* currentState = deepCopy(head);
    history.push({ currentState, {wasInsert, value} });

    deleteList(head);
    head = redoState;

    affectedPath.clear();
    search(value, affectedPath);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
    return affectedPath.empty() ? nullptr : affectedPath.back();
}

void LinkedList::clearHistory() {
    while (!history.empty()) {
        deleteList(history.top().first);
        history.pop();
    }
    while (!redoStack.empty()) {
        deleteList(redoStack.top().first);
        redoStack.pop();
    }
}

void LinkedList::generateRandom(int count, int minValue, int maxValue) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minValue, maxValue);

    for (int i = 0; i < count; ++i) {
        int randomValue = dis(gen);
        insert(randomValue);
    }
}

void LinkedList::calculatePositions(NodeL* node, int startX, int startY, int xOffset) {
    int x = startX;
    while (node) {
        node->targetX = static_cast<float>(x);
        node->targetY = static_cast<float>(startY);
        x += xOffset;
        node = node->next;
    }
}

void LinkedList::updateAnimation(float deltaTime) {
    NodeL* current = head;
    while (current) {
        if (instantMode) {
            current->x = current->targetX; // Instant position update
            current->y = current->targetY;
        }
        else {
            float dx = current->targetX - current->x;
            float dy = current->targetY - current->y;
            current->x += dx * deltaTime * 2.0f;
            current->y += dy * deltaTime * 2.0f;
        }
        current = current->next;
    }
}

void LinkedList::drawNode(NodeL* node, const std::vector<NodeL*>& highlightPath) {
    if (!node) return;

    Color color = { 100, 200, 150, 255 }; // Teal for default nodes
    bool isHighlighted = false;
    for (const NodeL* pathNode : highlightPath) {
        if (node == pathNode) {
            color = { 255, 165, 0, 255 }; // Orange for highlighted nodes
            isHighlighted = true;
            break;
        }
    }

    float pulse = sin(GetTime() * 5.0f) * 0.1f + 1.0f;
    float radius = isHighlighted ? 20 * pulse : 20;

    DrawCircle(static_cast<int>(node->x), static_cast<int>(node->y), radius, color);
    DrawCircleLines(static_cast<int>(node->x), static_cast<int>(node->y), radius, DARKGRAY);
    DrawText(std::to_string(node->value).c_str(), static_cast<int>(node->x) - 10, static_cast<int>(node->y) - 10, 20, BLACK);

    if (node->next) {
        DrawLine(static_cast<int>(node->x), static_cast<int>(node->y), static_cast<int>(node->next->x), static_cast<int>(node->next->y), LIGHTGRAY);
        drawNode(node->next, highlightPath);
    }
}

void LinkedList::draw(const std::vector<NodeL*>& highlightPath) {
    if (head) {
        drawNode(head, highlightPath);
    }
}

void LinkedList::LoadFromFile(std::string& searchResult) {
    const char* filters[] = { "*.txt" };
    const char* filePath = tinyfd_openFileDialog(
        "Select a Text File",
        "",
        1,
        filters,
        "Text Files",
        0
    );

    if (!filePath) {
        searchResult = "File selection canceled.";
        return;
    }

#ifdef _WIN32
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideFilePath = converter.from_bytes(filePath);
    FILE* file = nullptr;
    errno_t err = _wfopen_s(&file, wideFilePath.c_str(), L"r");
    if (err != 0 || file == nullptr) {
        searchResult = "Failed to open file: " + std::string(filePath);
        return;
    }
#else
    FILE* file = fopen(filePath, "r");
    if (!file) {
        searchResult = "Failed to open file: " + std::string(filePath);
        return;
    }
#endif

    clear();

    int value;
    int count = 0;
    if (instantMode) {
        while (fscanf_s(file, "%d", &value) == 1) {
            insert(value);
            calculatePositions(head, 50, GetScreenHeight() / 2, 100); // Update positions per insert
            updateAnimation(0.0f); // Force instant position update
            count++;
        }
        searchResult = "Instantly loaded " + std::to_string(count) + " values from " + std::string(filePath);
    }
    else {
        std::vector<NodeL*> path;
        while (fscanf_s(file, "%d", &value) == 1) {
            path.clear();
            head = insert(head, value, path);
            calculatePositions(head, 50, GetScreenHeight() / 2, 100);
            count++;
        }
        searchResult = "Loaded " + std::to_string(count) + " values from " + std::string(filePath);
    }

    fclose(file);
}

void runLinkedList() {
    const int screenWidth = 1400;
    const int screenHeight = 1000;

    LinkedList list;
    std::vector<NodeL*> searchPath;
    std::vector<NodeL*> insertPath;
    std::vector<NodeL*> affectedPath;
    char inputBuffer[10] = "";
    int inputIndex = 0;
    float searchTimer = 0.0f;
    int searchIndex = 0;
    bool searching = false;
    bool inserting = false;
    std::string searchResult = "";
    int lastSearchValue = 0;

    Rectangle insertButton = { 20, screenHeight - 120, 100, 40 };
    Rectangle deleteButton = { 130, screenHeight - 120, 100, 40 };
    Rectangle searchButton = { 240, screenHeight - 120, 100, 40 };
    Rectangle clearButton = { 350, screenHeight - 120, 100, 40 };
    Rectangle randomButton = { 460, screenHeight - 120, 100, 40 };
    Rectangle undoButton = { 570, screenHeight - 120, 100, 40 };
    Rectangle redoButton = { 680, screenHeight - 120, 100, 40 };
    Rectangle loadFileButton = { 790, screenHeight - 120, 100, 40 };
    Rectangle inputBox = { 20, screenHeight - 60, 100, 40 };
    Rectangle returnButton = { screenWidth - 120, 10, 100, 40 };

    Color TEAL = { 0, 128, 128, 255 };
    Color loadFileColor = { 0, 102, 204, 255 };
    Color instantColor = { 255, 105, 180, 255 }; // Pink for instant mode

    bool shouldReturn = false;

    while (!WindowShouldClose() && !shouldReturn) {
        float deltaTime = GetFrameTime();

        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= '0' && key <= '9') && inputIndex < 9) {
                inputBuffer[inputIndex++] = (char)key;
                inputBuffer[inputIndex] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && inputIndex > 0) {
            inputBuffer[--inputIndex] = '\0';
        }

        bool insertHover = CheckCollisionPointRec(GetMousePosition(), insertButton);
        bool deleteHover = CheckCollisionPointRec(GetMousePosition(), deleteButton);
        bool searchHover = CheckCollisionPointRec(GetMousePosition(), searchButton);
        bool clearHover = CheckCollisionPointRec(GetMousePosition(), clearButton);
        bool randomHover = CheckCollisionPointRec(GetMousePosition(), randomButton);
        bool undoHover = CheckCollisionPointRec(GetMousePosition(), undoButton);
        bool redoHover = CheckCollisionPointRec(GetMousePosition(), redoButton);
        bool loadFileHover = CheckCollisionPointRec(GetMousePosition(), loadFileButton);
        bool instantHover = CheckCollisionPointRec(GetMousePosition(), list.instantBtn);
        bool returnHover = CheckCollisionPointRec(GetMousePosition(), returnButton);

        bool insertClicked = isButtonClicked(insertButton);
        bool deleteClicked = isButtonClicked(deleteButton);
        bool searchClicked = isButtonClicked(searchButton);
        bool clearClicked = isButtonClicked(clearButton);
        bool randomClicked = isButtonClicked(randomButton);
        bool undoClicked = isButtonClicked(undoButton);
        bool redoClicked = isButtonClicked(redoButton);
        bool loadFileClicked = isButtonClicked(loadFileButton);
        bool instantClicked = isButtonClicked(list.instantBtn);
        bool returnClicked = isButtonClicked(returnButton);

        if (instantClicked) {
            list.instantMode = !list.instantMode;
            searchResult = list.instantMode ? "Instant Mode ON" : "Step-by-Step Mode ON";
            if (list.instantMode) {
                searching = false;
                inserting = false;
                searchPath.clear();
                insertPath.clear();
                searchTimer = 0.0f;
                searchIndex = 0;
            }
        }

        if (insertClicked && inputIndex > 0) {
            int value = atoi(inputBuffer);
            insertPath.clear();
            list.insert(value);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            if (!list.instantMode) {
                list.search(value, insertPath); // Update insertPath for animation
                searchPath = insertPath;
                searchIndex = 0;
                searchTimer = 0.0f;
                inserting = true;
            }
            else {
                searchPath.clear();
                inserting = false;
                list.updateAnimation(0.0f); // Force instant position update
            }
            searchResult = "";
        }
        if (deleteClicked && inputIndex > 0) {
            int value = atoi(inputBuffer);
            list.deleteNode(value);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
            if (list.instantMode) {
                list.updateAnimation(0.0f); // Force instant position update
            }
        }
        if (searchClicked && inputIndex > 0) {
            lastSearchValue = atoi(inputBuffer);
            list.search(lastSearchValue, searchPath);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            if (!list.instantMode) {
                searchIndex = 0;
                searchTimer = 0.0f;
                searching = true;
                searchResult = "Searching for " + std::to_string(lastSearchValue) + "...";
            }
            else {
                if (!searchPath.empty() && searchPath.back()->value == lastSearchValue) {
                    searchResult = "Node " + std::to_string(lastSearchValue) + " is found";
                }
                else {
                    searchResult = "Node " + std::to_string(lastSearchValue) + " is not found";
                }
                searching = false;
                searchPath.clear();
            }
        }
        if (clearClicked) {
            list.clear();
            searchPath.clear();
            insertPath.clear();
            affectedPath.clear();
            searching = false;
            inserting = false;
            searchResult = "";
        }
        if (randomClicked) {
            list.clear();
            list.generateRandom(10, 1, 100);
            searchPath.clear();
            insertPath.clear();
            affectedPath.clear();
            searching = false;
            inserting = false;
            searchResult = "";
            if (list.instantMode) {
                list.updateAnimation(0.0f); // Force instant position update
            }
        }
        if (undoClicked) {
            NodeL* affectedNode = list.undo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
            if (list.instantMode && affectedNode) {
                list.updateAnimation(0.0f); // Force instant position update
            }
        }
        if (redoClicked) {
            NodeL* affectedNode = list.redo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
            if (list.instantMode && affectedNode) {
                list.updateAnimation(0.0f); // Force instant position update
            }
        }
        if (loadFileClicked) {
            list.LoadFromFile(searchResult);
            searchPath.clear();
            inserting = false;
            searching = false;
        }
        if (returnClicked || IsKeyPressed(KEY_BACKSPACE)) {
            shouldReturn = true;
        }

        if ((searching || inserting) && !searchPath.empty() && !list.instantMode) {
            searchTimer += deltaTime;
            if (searchTimer >= 0.5f) {
                searchIndex++;
                searchTimer = 0.0f;
                if (searchIndex >= static_cast<int>(searchPath.size())) {
                    searchIndex = 0;
                    if (searching) {
                        if (!searchPath.empty() && searchPath.back()->value == lastSearchValue) {
                            searchResult = "Node " + std::to_string(lastSearchValue) + " is found";
                        }
                        else {
                            searchResult = "Node " + std::to_string(lastSearchValue) + " is not found";
                        }
                    }
                    searching = false;
                    inserting = false;
                }
            }
        }

        list.updateAnimation(deltaTime);

        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("Linked List Visualise", screenWidth / 2 - MeasureText("Linked List Visualise", 100) / 2, screenHeight / 4, 100, Fade(GRAY, 0.2f));

        std::vector<NodeL*> currentHighlight;
        if ((searching || inserting) && !list.instantMode) {
            if (searchIndex < static_cast<int>(searchPath.size())) {
                currentHighlight.push_back(searchPath[searchIndex]);
            }
        }
        else if (undoClicked || redoClicked) {
            if (!affectedPath.empty()) {
                currentHighlight.push_back(affectedPath.back());
            }
        }
        list.draw(currentHighlight);

        drawButton(insertButton, "Insert", GREEN, insertHover, insertClicked);
        drawButton(deleteButton, "Delete", RED, deleteHover, deleteClicked);
        drawButton(searchButton, "Search", BLUE, searchHover, searchClicked);
        drawButton(clearButton, "Clear", PURPLE, clearHover, clearClicked);
        drawButton(randomButton, "Random", ORANGE, randomHover, randomClicked);
        drawButton(undoButton, "Undo", GRAY, undoHover, undoClicked);
        drawButton(redoButton, "Redo", TEAL, redoHover, redoClicked);
        drawButton(loadFileButton, "File", loadFileColor, loadFileHover, loadFileClicked);
        drawButton(list.instantBtn, list.instantMode ? "Instant" : "Step", instantColor, instantHover, instantClicked);
        drawButton(returnButton, "Return", GRAY, returnHover, returnClicked);

        DrawRectangleRec(inputBox, LIGHTGRAY);
        DrawRectangleLinesEx(inputBox, 2, BLACK);
        DrawText(inputBuffer, inputBox.x + 5, inputBox.y + 10, 30, BLACK);
        DrawText("Enter number, then click action", inputBox.x, inputBox.y + 40, 20, DARKGRAY);
        DrawText(searchResult.c_str(), 20, screenHeight - 160, 20, DARKGRAY);

        EndDrawing();
    }
}