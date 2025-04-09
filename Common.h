#ifndef COMMON_H
#define COMMON_H
#include "raylib.h"

enum class AppState { MENU, AVL, HASH, GRAPH, LIST };

bool isButtonClicked(Rectangle button);
void drawButton(Rectangle button, const char* text, Color baseColor, bool hover, bool clicked = false);

#endif