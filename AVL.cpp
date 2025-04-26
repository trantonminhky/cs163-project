#include "raylib.h"
#include "avl.h"

enum class Screen {
    MENU,
    AVL_VISUALIZATION
};

struct AppState {
    Screen currentScreen;
    bool shouldExit;
};

bool runMenu(AppState& state) {
    const int screenWidth = 1400;
    const int screenHeight = 1000;

    Rectangle avlButton = { screenWidth / 2 - 150, screenHeight / 2 - 60, 300, 50 };
    Rectangle exitButton = { screenWidth / 2 - 150, screenHeight / 2 + 10, 300, 50 };

    while (!WindowShouldClose() && state.currentScreen == Screen::MENU) {
        bool avlHover = CheckCollisionPointRec(GetMousePosition(), avlButton);
        bool exitHover = CheckCollisionPointRec(GetMousePosition(), exitButton);

        bool avlClicked = isButtonClicked(avlButton);
        bool exitClicked = isButtonClicked(exitButton);

        if (avlClicked) {
            state.currentScreen = Screen::AVL_VISUALIZATION;
        }
        if (exitClicked || IsKeyPressed(KEY_ESCAPE)) {
            state.shouldExit = true;
            return false;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("AVL Tree Visualization", screenWidth / 2 - MeasureText("AVL Tree Visualization", 40) / 2, screenHeight / 2 - 150, 40, BLACK);
        drawButton(avlButton, "Start AVL Visualization", GREEN, avlHover, avlClicked);
        drawButton(exitButton, "Exit", RED, exitHover, exitClicked);
        DrawText("Press ESC to exit", screenWidth / 2 - MeasureText("Press ESC to exit", 20) / 2, screenHeight / 2 + 100, 20, DARKGRAY);
        EndDrawing();
    }

    return true;
}

int main() {
    InitWindow(1400, 1000, "AVL Tree Visualization");
    SetTargetFPS(60);

    AppState state;
    state.currentScreen = Screen::MENU;
    state.shouldExit = false;

    while (!state.shouldExit) {
        if (state.currentScreen == Screen::MENU) {
            if (!runMenu(state)) {
                break;
            }
        }
        else if (state.currentScreen == Screen::AVL_VISUALIZATION) {
            AVLTree tree;
            tree.runAVL();
            state.currentScreen = Screen::MENU;
        }
    }

    CloseWindow();
    return 0;
}