#include "AVL.h"
#include "raylib.h"
#include <cmath>
#include <string>
#include <random>

Node::Node(int value) : key(value), height(1), left(nullptr), right(nullptr), x(0), y(0), targetX(0), targetY(0), isDying(false) {}

AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree() {
    deleteTree(root);
    clearHistory(); // Ensure all trees in history and redoStack are deleted
}

void AVLTree::deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

Node* AVLTree::deepCopy(Node* node) {
    if (!node) return nullptr;
    Node* newNode = new Node(node->key);
    newNode->height = node->height;
    newNode->x = node->x;
    newNode->y = node->y;
    newNode->targetX = node->targetX;
    newNode->targetY = node->targetY;
    newNode->isDying = node->isDying;
    newNode->left = deepCopy(node->left);
    newNode->right = deepCopy(node->right);
    return newNode;
}

int AVLTree::getHeight(Node* node) {
    return node ? node->height : 0;
}

int AVLTree::getBalance(Node* node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void AVLTree::updateHeight(Node* node) {
    if (node) {
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    }
}

Node* AVLTree::rightRotate(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

Node* AVLTree::leftRotate(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

Node* AVLTree::insert(Node* node, int key, std::vector<Node*>& path) {
    if (!node) {
        Node* newNode = new Node(key);
        path.push_back(newNode);
        return newNode;
    }

    path.push_back(node);
    if (key < node->key)
        node->left = insert(node->left, key, path);
    else if (key > node->key)
        node->right = insert(node->right, key, path);
    else
        return node;

    updateHeight(node);
    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key) return rightRotate(node);
    if (balance < -1 && key > node->right->key) return leftRotate(node);
    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void AVLTree::insert(int key) {
    Node* treeCopy = deepCopy(root);
    history.push({ treeCopy, {true, key} });
    while (!redoStack.empty()) {
        deleteTree(redoStack.top().first);
        redoStack.pop();
    }

    std::vector<Node*> path;
    root = insert(root, key, path);
    calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
}

Node* AVLTree::findMin(Node* node) {
    while (node && node->left)
        node = node->left;
    return node;
}

Node* AVLTree::deleteNode(Node* node, int key) {
    if (!node) return nullptr;

    if (key < node->key)
        node->left = deleteNode(node->left, key);
    else if (key > node->key)
        node->right = deleteNode(node->right, key);
    else {
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        else if (!node->left) {
            Node* temp = node->right;
            delete node;
            return temp;
        }
        else if (!node->right) {
            Node* temp = node->left;
            delete node;
            return temp;
        }
        else {
            Node* temp = findMin(node->right);
            node->key = temp->key;
            node->right = deleteNode(node->right, temp->key);
        }
    }

    if (node) {
        updateHeight(node);
        int balance = getBalance(node);

        if (balance > 1 && getBalance(node->left) >= 0) return rightRotate(node);
        if (balance > 1 && getBalance(node->left) < 0) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && getBalance(node->right) <= 0) return leftRotate(node);
        if (balance < -1 && getBalance(node->right) > 0) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
    }

    return node;
}

void AVLTree::deleteNode(int key) {
    if (root) {
        Node* treeCopy = deepCopy(root);
        history.push({ treeCopy, {false, key} });
        while (!redoStack.empty()) {
            deleteTree(redoStack.top().first);
            redoStack.pop();
        }

        root = deleteNode(root, key);
        calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
    }
}

void AVLTree::clear() {
    deleteTree(root);
    root = nullptr;
    clearHistory();
}

Node* AVLTree::undo(std::vector<Node*>& affectedPath) {
    if (history.empty()) return nullptr;
    std::pair<Node*, std::pair<bool, int>> operation = history.top();
    Node* previousState = operation.first;
    bool wasInsert = operation.second.first;
    int value = operation.second.second;
    history.pop();

    // Store the current state for redo
    Node* currentState = deepCopy(root);
    redoStack.push({ currentState, {wasInsert, value} });

    // Delete the current tree and restore the previous state
    deleteTree(root);
    root = previousState;

    affectedPath.clear();
    search(value, affectedPath);
    calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
    return affectedPath.empty() ? nullptr : affectedPath.back();
}

Node* AVLTree::redo(std::vector<Node*>& affectedPath) {
    if (redoStack.empty()) return nullptr;
    std::pair<Node*, std::pair<bool, int>> operation = redoStack.top();
    Node* redoState = operation.first;
    bool wasInsert = operation.second.first;
    int value = operation.second.second;
    redoStack.pop();

    // Store the current state for undo
    Node* currentState = deepCopy(root);
    history.push({ currentState, {wasInsert, value} });

    // Delete the current tree and restore the redo state
    deleteTree(root);
    root = redoState;

    affectedPath.clear();
    search(value, affectedPath);
    calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
    return affectedPath.empty() ? nullptr : affectedPath.back();
}

void AVLTree::clearHistory() {
    while (!history.empty()) {
        deleteTree(history.top().first);
        history.pop();
    }
    while (!redoStack.empty()) {
        deleteTree(redoStack.top().first);
        redoStack.pop();
    }
}

void AVLTree::search(int key, std::vector<Node*>& searchPath) {
    searchPath.clear();
    Node* current = root;
    while (current) {
        searchPath.push_back(current);
        if (key == current->key) break;
        current = key < current->key ? current->left : current->right;
    }
}

void AVLTree::calculatePositions(Node* node, int x, int y, int xOffset, int depth) {
    if (!node) return;

    node->targetX = static_cast<float>(x);
    node->targetY = static_cast<float>(y);

    if (node->left)
        calculatePositions(node->left, x - std::max(50, xOffset / (depth + 1)), y + 80, xOffset, depth + 1);
    if (node->right)
        calculatePositions(node->right, x + std::max(50, xOffset / (depth + 1)), y + 80, xOffset, depth + 1);
}

void AVLTree::updateAnimation(float deltaTime) {
    std::vector<Node*> nodes;
    std::vector<Node*> stack = { root };

    while (!stack.empty()) {
        Node* node = stack.back();
        stack.pop_back();
        if (node) {
            nodes.push_back(node);
            if (node->left) stack.push_back(node->left);
            if (node->right) stack.push_back(node->right);
        }
    }

    for (Node* node : nodes) {
        float dx = node->targetX - node->x;
        float dy = node->targetY - node->y;
        node->x += dx * deltaTime * 5.0f;
        node->y += dy * deltaTime * 5.0f;
    }
}

void AVLTree::drawNode(Node* node, const std::vector<Node*>& highlightPath) {
    if (!node) return;

    Color color = BLUE;
    bool isHighlighted = false;
    for (const Node* pathNode : highlightPath) {
        if (node == pathNode) {
            color = YELLOW;
            isHighlighted = true;
            break;
        }
    }

    float pulse = sin(GetTime() * 5.0f) * 0.1f + 1.0f;
    float radius = isHighlighted ? 20 * pulse : 20;

    DrawCircle(static_cast<int>(node->x), static_cast<int>(node->y), radius, color);
    DrawText(std::to_string(node->key).c_str(), static_cast<int>(node->x) - 10, static_cast<int>(node->y) - 10, 20, WHITE);

    if (node->left) {
        DrawLine(static_cast<int>(node->x), static_cast<int>(node->y), static_cast<int>(node->left->x), static_cast<int>(node->left->y), LIGHTGRAY);
        drawNode(node->left, highlightPath);
    }
    if (node->right) {
        DrawLine(static_cast<int>(node->x), static_cast<int>(node->y), static_cast<int>(node->right->x), static_cast<int>(node->right->y), LIGHTGRAY);
        drawNode(node->right, highlightPath);
    }
}

void AVLTree::draw(const std::vector<Node*>& highlightPath) {
    if (root) {
        drawNode(root, highlightPath);
    }
}

void AVLTree::generateRandom(int count, int minValue, int maxValue) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minValue, maxValue);

    for (int i = 0; i < count; ++i) {
        int randomKey = dis(gen);
        insert(randomKey);
    }
}