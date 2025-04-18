#include "Graph.h"
#include "Common.h"
#include <cstdlib>
#include <ctime>
#include <raymath.h>
#include <cstring>
#include <algorithm>
#include <fstream>

GraphApp::GraphApp()
{
    screenWidth = 1400;
    screenHeight = 1000;
    srand(static_cast<unsigned>(time(nullptr)));
    inputText[0] = '\0';
    showInputBox = false;
    currentState = NORMAL;
    draggedVertex = -1;
    searchResult = "";
    showSearchMessage = false;
    kruskalTimer = 0.0f;
    kruskalStep = -1;
    showTable = false;
    totalWeight = 0;
    instantMode = false;
    instantBtn = { 710, static_cast<float>(screenHeight - 80), 140, 30 }; // Adjusted to bottom row
}

void GraphApp::Run()
{
    while (!WindowShouldClose() && !IsKeyPressed(KEY_BACKSPACE))
    {
        Update();
        bool dummy = false;
        Draw(dummy);
    }
}

void GraphApp::Update()
{
    Vector2 mousePos = GetMousePosition();

    for (Vertex& v : vertices)
    {
        v.scale = Lerp(v.scale, 1.0f, instantMode ? 1.0f : 0.1f);
        v.alpha = Lerp(v.alpha, 1.0f, instantMode ? 1.0f : 0.05f);
        v.position = Vector2Lerp(v.position, v.targetPos, instantMode ? 1.0f : 0.1f);
    }

    if (showInputBox)
    {
        int key = GetCharPressed();
        int len = strlen(inputText);
        while (key > 0)
        {
            if (((key >= 48 && key <= 57) || key == 32) && len < 31)
            {
                inputText[len] = static_cast<char>(key);
                inputText[len + 1] = '\0';
                len++;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
        {
            inputText[len - 1] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER))
        {
            ProcessInput();
            showInputBox = false;
            currentState = NORMAL;
        }
    }

    if (currentState == NORMAL || currentState == DRAGGING || showInputBox || currentState == KRUSKAL || currentState == INSERT || currentState == DELETE)
    {
        if (CheckCollisionPointRec(mousePos, { 20, static_cast<float>(screenHeight - 120), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (!redoStack.empty())
            {
                undoStack.push_back({ vertices, edges });
                GraphState nextState = redoStack.back();
                vertices = nextState.vertices;
                edges = nextState.edges;
                redoStack.pop_back();
            }
        }
        else if (CheckCollisionPointRec(mousePos, { 170, static_cast<float>(screenHeight - 120), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (!undoStack.empty())
            {
                redoStack.push_back({ vertices, edges });
                GraphState prevState = undoStack.back();
                vertices = prevState.vertices;
                edges = prevState.edges;
                undoStack.pop_back();
            }
        }
        else if (CheckCollisionPointRec(mousePos, { 320, static_cast<float>(screenHeight - 120), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            ResetGraphState();
            ProcessFileInput();
            currentState = NORMAL;
        }
        else if (CheckCollisionPointRec(mousePos, { 470, static_cast<float>(screenHeight - 120), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            undoStack.push_back({ vertices, edges });
            redoStack.clear();
            vertices.clear();
            edges.clear();
        }
        else if (CheckCollisionPointRec(mousePos, { 20, static_cast<float>(screenHeight - 80), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            ResetGraphState();
            currentState = RANDOM;
            searchResult = "";
            showSearchMessage = false;
            ProcessInput();
            currentState = NORMAL;
        }
        else if (CheckCollisionPointRec(mousePos, { 470, static_cast<float>(screenHeight - 80), 220, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            ResetGraphState();
            currentState = KRUSKAL;
            kruskalTimer = 0.0f;
            kruskalStep = 0;
            mstEdges.clear();
            totalWeight = 0;
            if (instantMode) {
                while (kruskalStep < static_cast<int>(edges.size())) {
                    RunKruskalStepByStep();
                }
                currentState = NORMAL;
                showTable = true;
            }
            else {
                RunKruskalStepByStep();
            }
        }
        else if (CheckCollisionPointRec(mousePos, { 320, static_cast<float>(screenHeight - 80), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (currentState == INSERT && showInputBox)
            {
                ProcessInsert();
                showInputBox = false;
                currentState = NORMAL;
            }
            else
            {
                ResetGraphState();
                currentState = INSERT;
                showInputBox = true;
                inputText[0] = '\0';
            }
        }
        else if (CheckCollisionPointRec(mousePos, { 170, static_cast<float>(screenHeight - 80), 140, 30 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (currentState == DELETE && showInputBox)
            {
                ProcessDelete();
                showInputBox = false;
                currentState = NORMAL;
            }
            else
            {
                ResetGraphState();
                currentState = DELETE;
                showInputBox = true;
                inputText[0] = '\0';
            }
        }
        else if (CheckCollisionPointRec(mousePos, instantBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            instantMode = !instantMode;
            searchResult = instantMode ? "Instant Mode ON" : "Step-by-Step Mode ON";
        }
        else if ((currentState == INSERT || currentState == DELETE) && showInputBox && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            showInputBox = false;
            currentState = NORMAL;
        }
    }

    if (currentState == KRUSKAL && !instantMode)
    {
        kruskalTimer += GetFrameTime();
        if (kruskalTimer >= 1.0f)
        {
            kruskalTimer = 0.0f;
            RunKruskalStepByStep();
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        if (currentState == NORMAL)
        {
            draggedVertex = GetVertexAt(mousePos);
            if (draggedVertex != -1)
            {
                currentState = DRAGGING;
                vertices[draggedVertex].scale = 1.2f;
            }
        }
        if (currentState == DRAGGING && draggedVertex != -1)
        {
            Vector2 newPos = mousePos;
            if (!WouldCollide(draggedVertex, newPos))
            {
                vertices[draggedVertex].targetPos = newPos;
            }
        }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && currentState == DRAGGING)
    {
        currentState = NORMAL;
        draggedVertex = -1;
    }

    const float k = 15000.0f;
    const float k_ui = 20000.0f;
    const float damping = 0.3f;
    vector<Vector2> forces(vertices.size(), { 0, 0 });

    for (size_t i = 0; i < vertices.size(); i++)
    {
        for (size_t j = i + 1; j < vertices.size(); j++)
        {
            Vector2 diff = Vector2Subtract(vertices[i].position, vertices[j].position);
            float distance = Vector2Length(diff);
            if (distance < 1.0f)
                distance = 1.0f;

            float forceMagnitude = k / (distance * distance);
            Vector2 force = Vector2Normalize(diff);
            force = Vector2Scale(force, forceMagnitude);

            forces[i] = Vector2Add(forces[i], force);
            forces[j] = Vector2Subtract(forces[j], force);
        }
    }

    for (size_t i = 0; i < vertices.size(); i++)
    {
        Rectangle uiElements[] = {
            {320, static_cast<float>(screenHeight - 120), 140, 30}, // File
            {20, static_cast<float>(screenHeight - 120), 140, 30},  // Redo
            {170, static_cast<float>(screenHeight - 120), 140, 30}, // Undo
            {470, static_cast<float>(screenHeight - 120), 140, 30}, // Delete All
            {20, static_cast<float>(screenHeight - 80), 140, 30},   // Random
            {470, static_cast<float>(screenHeight - 80), 220, 30},  // Kruskal
            {320, static_cast<float>(screenHeight - 80), 140, 30},  // Insert
            {170, static_cast<float>(screenHeight - 80), 140, 30},  // Delete
            {710, static_cast<float>(screenHeight - 80), 140, 30},  // Instant
            {static_cast<float>(screenWidth - 200), 90, 180, 40},   // Table
            {10, 10, static_cast<float>(MeasureText("MINIMUM SPANNING TREE", 30)), 30}, // Title
            {static_cast<float>(screenWidth - 150), 20, 120, 40},   // Return
            {870, static_cast<float>(screenHeight - 80), 150, 30}   // Input box
        };

        for (const Rectangle& ui : uiElements)
        {
            if (ui.x == screenWidth - 200 && ui.y == 90 && !showTable)
                continue;

            Vector2 closestPoint = {
                Clamp(vertices[i].position.x, ui.x, ui.x + ui.width),
                Clamp(vertices[i].position.y, ui.y, ui.y + ui.height) };
            Vector2 diff = Vector2Subtract(vertices[i].position, closestPoint);
            float distance = Vector2Length(diff);

            if (distance < 60.0f)
            {
                if (distance < 1.0f)
                    distance = 1.0f;
                float forceMagnitude = k_ui / (distance * distance);
                Vector2 force = (distance > 0.1f) ? Vector2Normalize(diff) : Vector2{ 1.0f, 0.0f };
                force = Vector2Scale(force, forceMagnitude);
                forces[i] = Vector2Add(forces[i], force);
            }
        }
    }

    for (size_t i = 0; i < vertices.size(); i++)
    {
        if (static_cast<int>(i) != draggedVertex)
        {
            Vector2 velocity = Vector2Scale(forces[i], damping);
            vertices[i].targetPos = Vector2Add(vertices[i].targetPos, velocity);
            vertices[i].targetPos.x = Clamp(vertices[i].targetPos.x, 50.0f, screenWidth - 50.0f);
            vertices[i].targetPos.y = Clamp(vertices[i].targetPos.y, 50.0f, screenHeight - 50.0f);
        }
    }
}

void GraphApp::Draw(bool& shouldReturn)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("MINIMUM SPANNING TREE", 10, 10, 30, BLACK);
    DrawText("Graph Visualise", screenWidth / 2 - MeasureText("Graph Visualise", 100) / 2, screenHeight / 2 - 50, 100, Fade(GRAY, 0.2f));

    Rectangle redoButton = { 20, static_cast<float>(screenHeight - 120), 140, 30 };
    Rectangle undoButton = { 170, static_cast<float>(screenHeight - 120), 140, 30 };
    Rectangle fileButton = { 320, static_cast<float>(screenHeight - 120), 140, 30 };
    Rectangle deleteAllButton = { 470, static_cast<float>(screenHeight - 120), 140, 30 };
    Rectangle randomButton = { 20, static_cast<float>(screenHeight - 80), 140, 30 };
    Rectangle kruskalButton = { 470, static_cast<float>(screenHeight - 80), 220, 30 };
    Rectangle insertButton = { 320, static_cast<float>(screenHeight - 80), 140, 30 };
    Rectangle deleteButton = { 170, static_cast<float>(screenHeight - 80), 140, 30 };

    bool redoHover = CheckCollisionPointRec(GetMousePosition(), redoButton);
    bool undoHover = CheckCollisionPointRec(GetMousePosition(), undoButton);
    bool fileHover = CheckCollisionPointRec(GetMousePosition(), fileButton);
    bool deleteAllHover = CheckCollisionPointRec(GetMousePosition(), deleteAllButton);
    bool randomHover = CheckCollisionPointRec(GetMousePosition(), randomButton);
    bool kruskalHover = CheckCollisionPointRec(GetMousePosition(), kruskalButton);
    bool insertHover = CheckCollisionPointRec(GetMousePosition(), insertButton);
    bool deleteHover = CheckCollisionPointRec(GetMousePosition(), deleteButton);
    bool instantHover = CheckCollisionPointRec(GetMousePosition(), instantBtn);

    bool redoClicked = isButtonClicked(redoButton);
    bool undoClicked = isButtonClicked(undoButton);
    bool fileClicked = isButtonClicked(fileButton);
    bool deleteAllClicked = isButtonClicked(deleteAllButton);
    bool randomClicked = isButtonClicked(randomButton);
    bool kruskalClicked = isButtonClicked(kruskalButton);
    bool insertClicked = isButtonClicked(insertButton);
    bool deleteClicked = isButtonClicked(deleteButton);
    bool instantClicked = isButtonClicked(instantBtn);

    Color TEAL = { 0, 128, 128, 255 };
    Color instantColor = instantMode ? PINK : LIME;
    drawButton(redoButton, "Redo", GRAY, redoHover, redoClicked);
    drawButton(undoButton, "Undo", DARKGRAY, undoHover, undoClicked);
    drawButton(fileButton, "Input File", Color{ 0, 102, 204, 255 }, fileHover, fileClicked);
    drawButton(deleteAllButton, "Delete All", RED, deleteAllHover, deleteAllClicked);
    drawButton(randomButton, "Random", ORANGE, randomHover, randomClicked);
    drawButton(kruskalButton, "Kruskal's Algorithm", PURPLE, kruskalHover, kruskalClicked);
    drawButton(insertButton, "Insert", GREEN, insertHover, insertClicked);
    drawButton(deleteButton, "Delete", PINK, deleteHover, deleteClicked);
    drawButton(instantBtn, instantMode ? "Instant" : "Step", instantColor, instantHover, instantClicked);

    if (showInputBox)
    {
        DrawRectangle(870, screenHeight - 80, 150, 30, LIGHTGRAY);
        DrawText(inputText, 875, screenHeight - 75, 20, BLACK);
    }

    if (showTable)
    {
        DrawRectangle(screenWidth - 200, 90, 180, 40, LIGHTGRAY);
        string totalText = "Total weight: " + to_string(totalWeight);
        DrawText(totalText.c_str(), screenWidth - 190, 100, 20, BLACK);
    }

    for (const Edge& e : edges)
    {
        Vector2 start = vertices[e.from].position;
        Vector2 end = vertices[e.to].position;
        Color lineColor = BLACK;
        Color textColor = BLACK;

        if (e.highlighted)
        {
            lineColor = RED;
            textColor = RED;
        }
        else if (e.blurred)
        {
            lineColor = { 128, 128, 128, 100 };
            textColor = { 128, 128, 128, 100 };
        }

        DrawLineV(start, end, lineColor);
        Vector2 midPoint = { (start.x + end.x) / 2, (start.y + end.y) / 2 };
        string weightStr = to_string(e.weight);
        int fontSize = 20;
        int textWidth = MeasureText(weightStr.c_str(), fontSize);
        int textX = midPoint.x - textWidth / 2;
        int textY = midPoint.y - 30;

        for (size_t i = 0; i < weightStr.length(); i++)
        {
            string digit = weightStr.substr(i, 1);
            int digitWidth = MeasureText(digit.c_str(), fontSize);
            DrawText(digit.c_str(), textX, textY, fontSize, textColor);
            textX += digitWidth + 2;
        }
    }

    for (const Vertex& v : vertices)
    {
        Color fadedColor = { BLUE.r, BLUE.g, BLUE.b, static_cast<unsigned char>(v.alpha * 255) };
        DrawCircleV(v.position, 30 * v.scale, fadedColor);
        string idStr = to_string(v.id);
        int numDigits = idStr.length();
        int fontSize = (numDigits > 3) ? 60 / numDigits : 20;
        fontSize = (fontSize < 10) ? 10 : fontSize;
        int textWidth = MeasureText(idStr.c_str(), fontSize);
        int textX = v.position.x - textWidth / 2;
        int textY = v.position.y - fontSize / 2;
        DrawText(idStr.c_str(), textX, textY, fontSize, WHITE);
    }

    Rectangle returnButton = { screenWidth - 150, 20, 120, 40 };
    bool returnHover = CheckCollisionPointRec(GetMousePosition(), returnButton);
    bool returnClicked = isButtonClicked(returnButton);
    drawButton(returnButton, "Return", TEAL, returnHover, returnClicked);
    if (returnClicked || IsKeyPressed(KEY_BACKSPACE)) {
        shouldReturn = true;
    }

    EndDrawing();
}

void GraphApp::HandleInitialize()
{
    vertices.clear();
    edges.clear();
    GenerateRandomGraph();
}

void GraphApp::GenerateRandomGraph()
{
    int numVertices = 5 + rand() % 6;
    for (int i = 0; i < numVertices; i++)
    {
        Vector2 pos = FindVacantPosition();
        vertices.push_back({ pos, i, false, 1.0f, 0.0f, pos });
    }

    vector<bool> visited(numVertices, false);
    visited[0] = true;
    int connectedCount = 1;

    while (connectedCount < numVertices)
    {
        int from = rand() % numVertices;
        int to = rand() % numVertices;
        int weight = 1 + rand() % 10;

        if (from != to && visited[from] && !visited[to] && !EdgeExists(from, to))
        {
            edges.push_back({ from, to, weight, false, false });
            visited[to] = true;
            connectedCount++;
        }
    }

    int extraEdges = rand() % numVertices;
    int attempts = 0;
    const int maxAttempts = extraEdges * 10;

    while (edges.size() < static_cast<size_t>(connectedCount - 1 + extraEdges) && attempts < maxAttempts)
    {
        int from = rand() % numVertices;
        int to = rand() % numVertices;
        int weight = 1 + rand() % 10;

        if (from != to && !EdgeExists(from, to))
        {
            edges.push_back({ from, to, weight, false, false });
        }
        attempts++;
    }
}

Vector2 GraphApp::FindVacantPosition()
{
    Vector2 pos;
    int attempts = 0;
    const int maxAttempts = 50;
    do
    {
        pos = { (float)(100 + rand() % (screenWidth - 200)),
               (float)(100 + rand() % (screenHeight - 200)) };
        attempts++;
        if (attempts > maxAttempts)
        {
            for (float x = 100; x < screenWidth - 100; x += 100)
            {
                for (float y = 100; y < screenHeight - 100; y += 100)
                {
                    Vector2 testPos = { x, y };
                    if (!WouldCollide(-1, testPos))
                    {
                        return testPos;
                    }
                }
            }
            return { screenWidth / 2.0f, screenHeight / 2.0f };
        }
    } while (WouldCollide(-1, pos));
    return pos;
}

bool GraphApp::WouldCollide(int excludeIndex, Vector2 newPos)
{
    const float vertexRadius = 30.0f;
    const float minDistance = vertexRadius * 2;

    for (size_t i = 0; i < vertices.size(); i++)
    {
        if (static_cast<int>(i) != excludeIndex)
        {
            if (Vector2Distance(newPos, vertices[i].position) < minDistance)
            {
                return true;
            }
        }
    }

    const int fontSize = 20;
    const float textBuffer = 10.0f;
    for (const Edge& e : edges)
    {
        Vector2 start = vertices[e.from].position;
        Vector2 end = vertices[e.to].position;
        Vector2 midPoint = { (start.x + end.x) / 2, (start.y + end.y) / 2 };
        string weightStr = to_string(e.weight);
        int textWidth = MeasureText(weightStr.c_str(), fontSize);
        float textHeight = fontSize;

        Rectangle textBox = {
            midPoint.x - textWidth / 2 - textBuffer,
            midPoint.y - 30 - textHeight - textBuffer,
            static_cast<float>(textWidth) + 2 * textBuffer,
            textHeight + 2 * textBuffer };

        if (CheckCollisionPointRec(newPos, textBox))
        {
            return true;
        }
    }
    return false;
}

bool GraphApp::EdgeExists(int fromIndex, int toIndex)
{
    for (const Edge& e : edges)
    {
        if ((e.from == fromIndex && e.to == toIndex) || (e.from == toIndex && e.to == fromIndex))
        {
            return true;
        }
    }
    return false;
}

void GraphApp::ProcessInput()
{
    if (strlen(inputText) > 0 || currentState == RANDOM)
    {
        if (currentState == RANDOM)
        {
            HandleInitialize();
        }
        else if (currentState == INSERT)
        {
            ProcessInsert();
        }
        else if (currentState == DELETE)
        {
            ProcessDelete();
        }
    }
}

void GraphApp::ProcessInsert()
{
    undoStack.push_back({ vertices, edges });
    redoStack.clear();

    string input(inputText);
    int from = -1, to = -1, weight = -1;
    int numbersFound = 0;
    string currentNumber = "";

    for (char c : input)
    {
        if (c == ' ')
        {
            if (!currentNumber.empty())
            {
                if (numbersFound == 0)
                    from = atoi(currentNumber.c_str());
                else if (numbersFound == 1)
                    to = atoi(currentNumber.c_str());
                else if (numbersFound == 2)
                    weight = atoi(currentNumber.c_str());
                numbersFound++;
                currentNumber = "";
            }
        }
        else if (c >= '0' && c <= '9')
        {
            currentNumber += c;
        }
    }
    if (!currentNumber.empty() && numbersFound < 3)
    {
        if (numbersFound == 0)
            from = atoi(currentNumber.c_str());
        else if (numbersFound == 1)
            to = atoi(currentNumber.c_str());
        else if (numbersFound == 2)
            weight = atoi(currentNumber.c_str());
        numbersFound++;
    }

    if (numbersFound == 3 && from != to && weight > 0)
    {
        bool fromExists = VertexExists(from);
        bool toExists = VertexExists(to);

        if (fromExists && toExists)
        {
            int fromIndex = GetVertexIndex(from);
            int toIndex = GetVertexIndex(to);
            if (!EdgeExists(fromIndex, toIndex))
            {
                edges.push_back({ fromIndex, toIndex, weight, false, false });
            }
        }
        else if (fromExists != toExists)
        {
            Vector2 pos = FindVacantPosition();
            int existingIndex, newIndex;
            if (fromExists)
            {
                existingIndex = GetVertexIndex(from);
                vertices.push_back({ pos, to, false, 1.0f, instantMode ? 1.0f : 0.0f, pos });
                newIndex = vertices.size() - 1;
                edges.push_back({ existingIndex, newIndex, weight, false, false });
            }
            else
            {
                existingIndex = GetVertexIndex(to);
                vertices.push_back({ pos, from, false, 1.0f, instantMode ? 1.0f : 0.0f, pos });
                newIndex = vertices.size() - 1;
                edges.push_back({ newIndex, existingIndex, weight, false, false });
            }
        }
        else if (!fromExists && !toExists)
        {
            Vector2 pos1 = FindVacantPosition();
            vertices.push_back({ pos1, from, false, 1.0f, instantMode ? 1.0f : 0.0f, pos1 });
            int fromIndex = vertices.size() - 1;

            Vector2 pos2 = FindVacantPosition();
            vertices.push_back({ pos2, to, false, 1.0f, instantMode ? 1.0f : 0.0f, pos2 });
            int toIndex = vertices.size() - 1;

            edges.push_back({ fromIndex, toIndex, weight, false, false });
        }
    }
}

void GraphApp::ProcessDelete()
{
    undoStack.push_back({ vertices, edges });
    redoStack.clear();

    string input(inputText);
    int num1 = -1, num2 = -1;
    int numbersFound = 0;
    string currentNumber = "";

    for (char c : input)
    {
        if (c == ' ')
        {
            if (!currentNumber.empty())
            {
                if (numbersFound == 0)
                    num1 = atoi(currentNumber.c_str());
                else if (numbersFound == 1)
                    num2 = atoi(currentNumber.c_str());
                numbersFound++;
                currentNumber = "";
            }
        }
        else if (c >= '0' && c <= '9')
        {
            currentNumber += c;
        }
    }
    if (!currentNumber.empty() && numbersFound < 2)
    {
        if (numbersFound == 0)
            num1 = atoi(currentNumber.c_str());
        else if (numbersFound == 1)
            num2 = atoi(currentNumber.c_str());
        numbersFound++;
    }

    if (numbersFound == 1)
    {
        if (!VertexExists(num1))
            return;
        int vertexIndex = GetVertexIndex(num1);

        for (auto it = edges.begin(); it != edges.end();)
        {
            if (it->from == vertexIndex || it->to == vertexIndex)
            {
                it = edges.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (auto it = vertices.begin(); it != vertices.end(); ++it)
        {
            if (it->id == num1)
            {
                vertices.erase(it);
                break;
            }
        }

        for (auto& edge : edges)
        {
            if (edge.from > vertexIndex)
                edge.from--;
            if (edge.to > vertexIndex)
                edge.to--;
        }
    }
    else if (numbersFound == 2)
    {
        if (!VertexExists(num1) || !VertexExists(num2))
            return;

        int fromIndex = GetVertexIndex(num1);
        int toIndex = GetVertexIndex(num2);
        bool edgeFound = false;
        for (auto it = edges.begin(); it != edges.end(); ++it)
        {
            if ((it->from == fromIndex && it->to == toIndex) || (it->from == toIndex && it->to == fromIndex))
            {
                edges.erase(it);
                edgeFound = true;
                break;
            }
        }
        if (!edgeFound)
            return;
    }
}

int GraphApp::GetVertexAt(Vector2 pos)
{
    for (size_t i = 0; i < vertices.size(); i++)
    {
        if (CheckCollisionPointCircle(pos, vertices[i].position, 30))
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool GraphApp::VertexExists(int id)
{
    for (const Vertex& v : vertices)
    {
        if (v.id == id)
            return true;
    }
    return false;
}

int GraphApp::GetVertexIndex(int id)
{
    for (size_t i = 0; i < vertices.size(); i++)
    {
        if (vertices[i].id == id)
            return static_cast<int>(i);
    }
    return -1;
}

void GraphApp::ResetGraphState()
{
    for (auto& edge : edges)
    {
        edge.highlighted = false;
        edge.blurred = false;
    }
    showTable = false;
    mstEdges.clear();
    totalWeight = 0;
    kruskalStep = -1;
}

void GraphApp::MakeSet(int v)
{
    parent[v] = v;
}

int GraphApp::Find(int v)
{
    if (v == parent[v])
        return v;
    return parent[v] = Find(parent[v]);
}

void GraphApp::Union(int a, int b)
{
    a = Find(a);
    b = Find(b);
    if (a != b)
        parent[b] = a;
}

void GraphApp::RunKruskalStepByStep()
{
    if (kruskalStep == 0)
    {
        parent.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++)
            MakeSet(i);
        sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b)
            { return a.weight < b.weight; });
    }

    if (kruskalStep < static_cast<int>(edges.size()))
    {
        Edge& e = edges[kruskalStep];
        if (Find(e.from) != Find(e.to))
        {
            Union(e.from, e.to);
            e.highlighted = true;
            mstEdges.push_back(e);
            totalWeight += e.weight;
        }
        else
        {
            e.blurred = true;
        }
        kruskalStep++;
    }
    else
    {
        currentState = NORMAL;
        showTable = true;
    }
}

void GraphApp::ProcessFileInput()
{
    using namespace std;

    const char* filePath = tinyfd_openFileDialog(
        "Select Graph Data File",
        "",
        0,
        NULL,
        NULL, 1);

    if (!filePath)
        return;

    ifstream file(filePath);
    if (!file.is_open())
        return;

    undoStack.push_back({ vertices, edges });
    redoStack.clear();

    string line;
    int count = 0;
    if (instantMode) {
        while (getline(file, line))
        {
            int from = -1, to = -1, weight = -1;
            int numbersFound = 0;
            string currentNumber;

            for (char c : line)
            {
                if (c == ' ')
                {
                    if (!currentNumber.empty())
                    {
                        if (numbersFound == 0)
                            from = atoi(currentNumber.c_str());
                        else if (numbersFound == 1)
                            to = atoi(currentNumber.c_str());
                        else if (numbersFound == 2)
                            weight = atoi(currentNumber.c_str());
                        numbersFound++;
                        currentNumber.clear();
                    }
                }
                else if (c >= '0' && c <= '9')
                {
                    currentNumber += c;
                }
            }
            if (!currentNumber.empty() && numbersFound < 3)
            {
                if (numbersFound == 0)
                    from = atoi(currentNumber.c_str());
                else if (numbersFound == 1)
                    to = atoi(currentNumber.c_str());
                else if (numbersFound == 2)
                    weight = atoi(currentNumber.c_str());
                numbersFound++;
            }

            if (numbersFound == 3 && from != to && weight > 0)
            {
                snprintf(inputText, sizeof(inputText), "%d %d %d", from, to, weight);
                currentState = INSERT;
                ProcessInsert();
                count++;
            }
        }
        searchResult = "Instantly loaded " + to_string(count) + " edges from " + string(filePath);
    }
    else {
        while (getline(file, line))
        {
            int from = -1, to = -1, weight = -1;
            int numbersFound = 0;
            string currentNumber;

            for (char c : line)
            {
                if (c == ' ')
                {
                    if (!currentNumber.empty())
                    {
                        if (numbersFound == 0)
                            from = atoi(currentNumber.c_str());
                        else if (numbersFound == 1)
                            to = atoi(currentNumber.c_str());
                        else if (numbersFound == 2)
                            weight = atoi(currentNumber.c_str());
                        numbersFound++;
                        currentNumber.clear();
                    }
                }
                else if (c >= '0' && c <= '9')
                {
                    currentNumber += c;
                }
            }
            if (!currentNumber.empty() && numbersFound < 3)
            {
                if (numbersFound == 0)
                    from = atoi(currentNumber.c_str());
                else if (numbersFound == 1)
                    to = atoi(currentNumber.c_str());
                else if (numbersFound == 2)
                    weight = atoi(currentNumber.c_str());
                numbersFound++;
            }

            if (numbersFound == 3 && from != to && weight > 0)
            {
                snprintf(inputText, sizeof(inputText), "%d %d %d", from, to, weight);
                currentState = INSERT;
                ProcessInsert();
                count++;
            }
        }
        searchResult = "Loaded " + to_string(count) + " edges from " + string(filePath);
    }

    file.close();
    currentState = NORMAL;
}

void runGraph()
{
    const int screenWidth = 1400;
    const int screenHeight = 1000;

    GraphApp app;
    bool shouldReturn = false;

    while (!WindowShouldClose() && !shouldReturn) {
        app.Update();
        app.Draw(shouldReturn);
    }
}