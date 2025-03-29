#include "LinkedListHeader.h"
#include "raylib.h"
#include <string>
#include <random>

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
    calculatePositions(head, 100, GetScreenHeight() / 2);
}

Node* LinkedList::deleteNode(Node* node, int value, std::vector<Node*>& affectedPath) {
    if (!node) return nullptr;
    affectedPath.push_back(node);
    if (node->value == value) {
        Node* temp = node->next;
        delete node;
        return temp;
    }
    node->next = deleteNode(node->next, value, affectedPath);
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

    std::vector<Node*> affectedPath;
    head = deleteNode(head, value, affectedPath);
    calculatePositions(head, 100, GetScreenHeight() / 2);
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
    auto operation = history.top();
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
    calculatePositions(head, 100, GetScreenHeight() / 2);
    return affectedPath.empty() ? nullptr : affectedPath.back();
}

Node* LinkedList::redo(std::vector<Node*>& affectedPath) {
    if (redoStack.empty()) return nullptr;
    auto operation = redoStack.top();
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
    calculatePositions(head, 100, GetScreenHeight() / 2);
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

void LinkedList::calculatePositions(Node* node, int startX, int startY) {
    int x = startX;
    Node* current = node;
    while (current) {
        current->targetX = static_cast<float>(x);
        current->targetY = static_cast<float>(startY);
        x += 100; // Fixed horizontal spacing to prevent overlap
        current = current->next;
    }
}

void LinkedList::updateAnimation(float deltaTime) {
    Node* current = head;
    while (current) {
        float dx = current->targetX - current->x;
        float dy = current->targetY - current->y;
        current->x += dx * deltaTime * 2.0f; // Slow animation (like AVL)
        current->y += dy * deltaTime * 2.0f;
        current = current->next;
    }
}

void LinkedList::drawNode(Node* node, const std::vector<Node*>& highlightPath) {
    if (!node) return;

    Color color = { 100, 200, 150, 255 }; // Soft teal (like AVL)
    bool isHighlighted = false;
    for (const Node* pathNode : highlightPath) {
        if (node == pathNode) {
            color = { 255, 165, 0, 255 }; // Bright orange (like AVL)
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
    if (head) drawNode(head, highlightPath);
}

void LinkedList::generateRandom(int count, int minValue, int maxValue) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minValue, maxValue);

    for (int i = 0; i < count; ++i) {
        insert(dis(gen));
    }
}