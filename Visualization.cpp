#include "raylib.h"
#include "Common.h"
#include "AVL.h"
#include "HashTable.h"
#include "Graph.h"
#include "LinkedList.h"

void runAVL();
void runHashTable();
void runGraph();
void runLinkedList();
bool runMenu(AppState& state);  // Changed to return bool

int main() {
    const int screenWidth = 1400;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "DSA Visualizer");
    SetTargetFPS(60);

    AppState currentState = AppState::MENU;

    while (!WindowShouldClose()) {
        switch (currentState) {
        case AppState::MENU:
            if (runMenu(currentState)) {  // Check if exit was requested
                CloseWindow();
                return 0;  // Exit the program
            }
            break;
        case AppState::AVL:
            runAVL();
            currentState = AppState::MENU;
            break;
        case AppState::HASH:
            runHashTable();
            currentState = AppState::MENU;
            break;
        case AppState::GRAPH:
            runGraph();
            currentState = AppState::MENU;
            break;
        case AppState::LIST:
            runLinkedList();
            currentState = AppState::MENU;
            break;
        }
    }

    CloseWindow();
    return 0;
}