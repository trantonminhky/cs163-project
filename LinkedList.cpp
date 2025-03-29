#include "raylib.h"
#include "LinkedListHeader.h"
#include <vector>
#include <string>
#include <iostream>

bool isButtonClicked(Rectangle button) {
    bool clicked = CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    return clicked;
}

void drawButton(Rectangle button, const char* text, Color baseColor, bool hover, bool clicked = false) {
    Color color = hover ? Fade(baseColor, 0.8f) : baseColor;
    if (clicked) color = Fade(baseColor, 0.6f);
    float scale = clicked ? 0.95f : (hover ? 1.05f : 1.0f);

    Rectangle scaledButton = {
        button.x + button.width * (1.0f - scale) / 2,
        button.y + button.height * (1.0f - scale) / 2,
        button.width * scale,
        button.height * scale
    };

    DrawRectangleRounded(scaledButton, 0.3f, 10, color);
    DrawRectangleLinesEx(scaledButton, 2, BLACK);

    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    DrawText(text,
        scaledButton.x + (scaledButton.width - textSize.x) / 2,
        scaledButton.y + (scaledButton.height - textSize.y) / 2,
        20, WHITE);
}

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "Linked List Visualization");
    if (!IsWindowReady()) {
        std::cout << "Failed to initialize window!" << std::endl;
        return -1;
    }
    SetTargetFPS(60);

    LinkedList list;
    std::vector<Node*> searchPath;
    std::vector<Node*> insertPath;
    std::vector<Node*> affectedPath;
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
    Rectangle inputBox = { 20, screenHeight - 60, 100, 40 };

    Color TEAL = { 0, 128, 128, 255 };

    while (!WindowShouldClose()) {
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

        bool insertClicked = isButtonClicked(insertButton);
        bool deleteClicked = isButtonClicked(deleteButton);
        bool searchClicked = isButtonClicked(searchButton);
        bool clearClicked = isButtonClicked(clearButton);
        bool randomClicked = isButtonClicked(randomButton);
        bool undoClicked = isButtonClicked(undoButton);
        bool redoClicked = isButtonClicked(redoButton);

        if (insertClicked && inputIndex > 0) {
            int value = atoi(inputBuffer);
            insertPath.clear();
            list.insert(value);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searchPath = insertPath;
            searchIndex = 0;
            searchTimer = 0.0f;
            inserting = true;
            searchResult = "";
        }
        if (deleteClicked && inputIndex > 0) {
            int value = atoi(inputBuffer);
            list.deleteNode(value);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searching = false;
            inserting = false;
            searchResult = "";
        }
        if (searchClicked && inputIndex > 0) {
            lastSearchValue = atoi(inputBuffer);
            list.search(lastSearchValue, searchPath);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searchIndex = 0;
            searchTimer = 0.0f;
            searching = true;
            searchResult = "Searching for " + std::to_string(lastSearchValue) + "...";
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
            list.generateRandom(8, 1, 100);
            searchPath.clear();
            insertPath.clear();
            affectedPath.clear();
            searching = false;
            inserting = false;
            searchResult = "";
        }
        if (undoClicked) {
            list.undo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
        }
        if (redoClicked) {
            list.redo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
        }

        if ((searching || inserting) && !searchPath.empty()) {
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

        // Draw background message
        const char* backgroundText = "Visualize Singly Linked List";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), backgroundText, 50, 1);
        DrawText(backgroundText,
            screenWidth / 2 - textSize.x / 2,
            screenHeight / 2 - textSize.y / 2,
            50,
            Fade(LIGHTGRAY, 0.3f)); // Faded to act as watermark

        std::vector<Node*> currentHighlight;
        if (searching || inserting) {
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

        DrawRectangleRec(inputBox, LIGHTGRAY);
        DrawRectangleLinesEx(inputBox, 2, BLACK);
        DrawText(inputBuffer, inputBox.x + 5, inputBox.y + 10, 30, BLACK);
        DrawText("Enter number, then click action", inputBox.x, inputBox.y + 40, 20, DARKGRAY);
        DrawText(searchResult.c_str(), 20, screenHeight - 160, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}