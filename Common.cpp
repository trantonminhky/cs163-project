#include "Common.h"

bool isButtonClicked(Rectangle button) {
    return CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void drawButton(Rectangle button, const char* text, Color baseColor, bool hover, bool clicked) {
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