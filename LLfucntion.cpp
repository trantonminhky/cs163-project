#include "LinkedList.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <random>
#include <stdio.h>

Node::Node(int val) : value(val), next(nullptr), x(0), y(0), targetX(0), targetY(0) {}

LinkedList::LinkedList() : head(nullptr) {}

LinkedList::~LinkedList() {
    deleteList(head);
    clearHistory();
}

void LinkedList::deleteList(Node* node) {
    while (node) {
        Node* temp = node;
        node = node->next;
        delete temp;
    }
}

Node* LinkedList::deepCopy(Node* node) {
    if (!node) return nullptr;
    Node* newNode = new Node(node->value);
    newNode->x = node->x;
    newNode->y = node->y;
    newNode->targetX = node->targetX;
    newNode->targetY = node->targetY;
    newNode->next = deepCopy(node->next);
    return newNode;
}

Node* LinkedList::insert(Node* node, int value, std::vector<Node*>& path) {
    if (!node) {
        Node* newNode = new Node(value);
        path.push_back(newNode);
        return newNode;
    }

    path.push_back(node);
    node->next = insert(node->next, value, path);
    return node;
}

void LinkedList::insert(int value) {
    Node* listCopy = deepCopy(head);
    history.push({ listCopy, {true, value} });
    while (!redoStack.empty()) {
        deleteList(redoStack.top().first);
        redoStack.pop();
    }

    std::vector<Node*> path;
    head = insert(head, value, path);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
}

Node* LinkedList::deleteNode(Node* node, int value) {
    if (!node) return nullptr;

    if (node->value == value) {
        Node* temp = node->next;
        delete node;
        return temp;
    }

    node->next = deleteNode(node->next, value);
    return node;
}

void LinkedList::deleteNode(int value) {
    if (!head) return;

    Node* listCopy = deepCopy(head);
    history.push({ listCopy, {false, value} });
    while (!redoStack.empty()) {
        deleteList(redoStack.top().first);
        redoStack.pop();
    }

    head = deleteNode(head, value);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
}

void LinkedList::search(int value, std::vector<Node*>& searchPath) {
    searchPath.clear();
    Node* current = head;
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

Node* LinkedList::undo(std::vector<Node*>& affectedPath) {
    if (history.empty()) return nullptr;
    std::pair<Node*, std::pair<bool, int>> operation = history.top();
    Node* previousState = operation.first;
    bool wasInsert = operation.second.first;
    int value = operation.second.second;
    history.pop();

    Node* currentState = deepCopy(head);
    redoStack.push({ currentState, {wasInsert, value} });

    deleteList(head);
    head = previousState;

    affectedPath.clear();
    search(value, affectedPath);
    calculatePositions(head, 50, GetScreenHeight() / 2, 100);
    return affectedPath.empty() ? nullptr : affectedPath.back();
}

Node* LinkedList::redo(std::vector<Node*>& affectedPath) {
    if (redoStack.empty()) return nullptr;
    std::pair<Node*, std::pair<bool, int>> operation = redoStack.top();
    Node* redoState = operation.first;
    bool wasInsert = operation.second.first;
    int value = operation.second.second;
    redoStack.pop();

    Node* currentState = deepCopy(head);
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

void LinkedList::calculatePositions(Node* node, int startX, int startY, int xOffset) {
    int x = startX;
    while (node) {
        node->targetX = static_cast<float>(x);
        node->targetY = static_cast<float>(startY);
        x += xOffset;
        node = node->next;
    }
}

void LinkedList::updateAnimation(float deltaTime) {
    Node* current = head;
    while (current) {
        float dx = current->targetX - current->x;
        float dy = current->targetY - current->y;
        current->x += dx * deltaTime * 2.0f;
        current->y += dy * deltaTime * 2.0f;
        current = current->next;
    }
}

void LinkedList::drawNode(Node* node, const std::vector<Node*>& highlightPath) {
    if (!node) return;

    Color color = { 100, 200, 150, 255 }; // Teal for default nodes
    bool isHighlighted = false;
    for (const Node* pathNode : highlightPath) {
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

void LinkedList::draw(const std::vector<Node*>& highlightPath) {
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
    while (fscanf_s(file, "%d", &value) == 1) {
        insert(value);
        count++;
    }

    fclose(file);
    searchResult = "Loaded " + std::to_string(count) + " values from " + std::string(filePath);
}