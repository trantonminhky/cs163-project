//#include "C:\raylib\raylib\src\raylib.h"
#include "../include/raylib.h"
#include "HashTable.h"
#include "UI.h"

int main() {
    const Color backgroundColor = {245, 245, 245, 255};
    
    constexpr int screenWidth = 1500;
    constexpr int screenHeight = 1000;
    
    HashTable hashTable;
    UI ui(&hashTable);
    
    InitWindow(screenWidth, screenHeight, "Hash Table Visualization");
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        ui.update();
        // ajsbdjsadnds 
        BeginDrawing();
            ClearBackground(backgroundColor);
            ui.draw();
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}