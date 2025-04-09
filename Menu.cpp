#include "raylib.h"
#include "Common.h"

bool runMenu(AppState& state) {
    Rectangle avlButton = { 600, 250, 200, 50 };
    Rectangle hashButton = { 600, 350, 200, 50 };
    Rectangle graphButton = { 600, 450, 200, 50 };
    Rectangle listButton = { 600, 550, 200, 50 };
    Rectangle exitButton = { 600, 650, 200, 50 };

    BeginDrawing();
    ClearBackground(RAYWHITE);


    DrawText("DSA Visualizer", 700 - MeasureText("DSA Visualizer", 60) / 2, 80, 60, BLACK);

    bool avlHover = CheckCollisionPointRec(GetMousePosition(), avlButton);
    bool hashHover = CheckCollisionPointRec(GetMousePosition(), hashButton);
    bool graphHover = CheckCollisionPointRec(GetMousePosition(), graphButton);
    bool listHover = CheckCollisionPointRec(GetMousePosition(), listButton);
    bool exitHover = CheckCollisionPointRec(GetMousePosition(), exitButton);

    drawButton(avlButton, "AVL Tree", GREEN, avlHover, isButtonClicked(avlButton));
    drawButton(hashButton, "Hash Table", BLUE, hashHover, isButtonClicked(hashButton));
    drawButton(graphButton, "Graph", PURPLE, graphHover, isButtonClicked(graphButton));
    drawButton(listButton, "Linked List", ORANGE, listHover, isButtonClicked(listButton));
    drawButton(exitButton, "Exit", RED, exitHover, isButtonClicked(exitButton));

    if (isButtonClicked(avlButton)) state = AppState::AVL;
    if (isButtonClicked(hashButton)) state = AppState::HASH;
    if (isButtonClicked(graphButton)) state = AppState::GRAPH;
    if (isButtonClicked(listButton)) state = AppState::LIST;
    if (isButtonClicked(exitButton)) {
        EndDrawing();
        return true;
    }

    EndDrawing();
    return false;
}