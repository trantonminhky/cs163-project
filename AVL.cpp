#include "raylib.h"
#include "AVL.h"
#include "Common.h"
#include <vector>
#include <string>
#include <iostream>
#include <random>

Node::Node(int value) : key(value), height(1), left(nullptr), right(nullptr), x(0), y(0), targetX(0), targetY(0), isDying(false) {}

AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree() {
    deleteTree(root);
    clearHistory();
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

    Node* currentState = deepCopy(root);
    redoStack.push({ currentState, {wasInsert, value} });

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

    Node* currentState = deepCopy(root);
    history.push({ currentState, {wasInsert, value} });

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

    int verticalSpacing = 100;
    int adjustedXOffset = xOffset / (depth > 1 ? depth : 1);
    adjustedXOffset = std::max(100, adjustedXOffset);

    if (node->left)
        calculatePositions(node->left, x - adjustedXOffset, y + verticalSpacing, xOffset, depth + 1);
    if (node->right)
        calculatePositions(node->right, x + adjustedXOffset, y + verticalSpacing, xOffset, depth + 1);
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
        node->x += dx * deltaTime * 2.0f;
        node->y += dy * deltaTime * 2.0f;
    }
}

void AVLTree::drawNode(Node* node, const std::vector<Node*>& highlightPath) {
    if (!node) return;

    Color color = { 100, 200, 150, 255 };
    bool isHighlighted = false;
    for (const Node* pathNode : highlightPath) {
        if (node == pathNode) {
            color = { 255, 165, 0, 255 };
            isHighlighted = true;
            break;
        }
    }

    float pulse = sin(GetTime() * 5.0f) * 0.1f + 1.0f;
    float radius = isHighlighted ? 20 * pulse : 20;

    DrawCircle(static_cast<int>(node->x), static_cast<int>(node->y), radius, color);
    DrawCircleLines(static_cast<int>(node->x), static_cast<int>(node->y), radius, DARKGRAY);
    DrawText(std::to_string(node->key).c_str(), static_cast<int>(node->x) - 10, static_cast<int>(node->y) - 10, 20, BLACK);

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

void AVLTree::LoadFromFile(std::string& searchResult) {
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

void runAVL() {
    const int screenWidth = 1400;
    const int screenHeight = 1000;

    AVLTree tree;
    std::vector<Node*> searchPath;
    std::vector<Node*> insertPath;
    std::vector<Node*> affectedPath;
    char inputBuffer[10] = "";
    int inputIndex = 0;
    float searchTimer = 0.0f;
    int searchIndex = 0;
    bool searching = false;
    bool inserting = false;
    std::string searchResult = "";
    int lastSearchValue = 0;

    Rectangle insertButton = { 20, screenHeight - 120, 100, 40 };
    Rectangle deleteButton = { 130, screenHeight - 120, 100, 40 };
    Rectangle searchButton = { 240, screenHeight - 120, 100, 40 };
    Rectangle clearButton = { 350, screenHeight - 120, 100, 40 };
    Rectangle randomButton = { 460, screenHeight - 120, 100, 40 };
    Rectangle undoButton = { 570, screenHeight - 120, 100, 40 };
    Rectangle redoButton = { 680, screenHeight - 120, 100, 40 };
    Rectangle FileButton = { 790, screenHeight - 120, 100, 40 };
    Rectangle inputBox = { 20, screenHeight - 60, 100, 40 };

    Rectangle returnButton = { screenWidth - 120, 10, 100, 40 };

    Color TEAL = { 0, 128, 128, 255 };
    Color Mediumblue = { 0, 102, 204, 255 };

    bool shouldReturn = false; // Flag to exit loop

    while (!WindowShouldClose() && !shouldReturn) {
        float deltaTime = GetFrameTime();

        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= '0' && key <= '9') && inputIndex < 9) {
                inputBuffer[inputIndex++] = (char)key;
                inputBuffer[inputIndex] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && inputIndex > 0) {
            inputBuffer[--inputIndex] = '\0';
        }

        bool insertHover = CheckCollisionPointRec(GetMousePosition(), insertButton);
        bool deleteHover = CheckCollisionPointRec(GetMousePosition(), deleteButton);
        bool searchHover = CheckCollisionPointRec(GetMousePosition(), searchButton);
        bool clearHover = CheckCollisionPointRec(GetMousePosition(), clearButton);
        bool randomHover = CheckCollisionPointRec(GetMousePosition(), randomButton);
        bool undoHover = CheckCollisionPointRec(GetMousePosition(), undoButton);
        bool redoHover = CheckCollisionPointRec(GetMousePosition(), redoButton);
        bool FileHover = CheckCollisionPointRec(GetMousePosition(), FileButton);
        bool returnHover = CheckCollisionPointRec(GetMousePosition(), returnButton);

        bool insertClicked = isButtonClicked(insertButton);
        bool deleteClicked = isButtonClicked(deleteButton);
        bool searchClicked = isButtonClicked(searchButton);
        bool clearClicked = isButtonClicked(clearButton);
        bool randomClicked = isButtonClicked(randomButton);
        bool undoClicked = isButtonClicked(undoButton);
        bool redoClicked = isButtonClicked(redoButton);
        bool FileClicked = isButtonClicked(FileButton);
        bool returnClicked = isButtonClicked(returnButton);

        if (insertClicked && inputIndex > 0) {
            int value = atoi(inputBuffer);
            insertPath.clear();
            tree.insert(value);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searchPath = insertPath;
            searchIndex = 0;
            searchTimer = 0.0f;
            inserting = true;
            searchResult = "";
        }
        if (deleteClicked && inputIndex > 0) {
            int value = atoi(inputBuffer);
            tree.deleteNode(value);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searching = false;
            inserting = false;
            searchResult = "";
        }
        if (searchClicked && inputIndex > 0) {
            lastSearchValue = atoi(inputBuffer);
            tree.search(lastSearchValue, searchPath);
            inputIndex = 0;
            inputBuffer[0] = '\0';
            searchIndex = 0;
            searchTimer = 0.0f;
            searching = true;
            searchResult = "Searching for " + std::to_string(lastSearchValue) + "...";
        }
        if (clearClicked) {
            tree.clear();
            searchPath.clear();
            insertPath.clear();
            affectedPath.clear();
            searching = false;
            inserting = false;
            searchResult = "";
        }
        if (randomClicked) {
            tree.clear();
            tree.generateRandom(10, 1, 100);
            searchPath.clear();
            insertPath.clear();
            affectedPath.clear();
            searching = false;
            inserting = false;
            searchResult = "";
        }
        if (undoClicked) {
            Node* affectedNode = tree.undo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
        }
        if (redoClicked) {
            Node* affectedNode = tree.redo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
        }
        if (FileClicked) {
            tree.LoadFromFile(searchResult);
            searchPath.clear();
            inserting = false;
            searching = false;
        }
        if (returnClicked || IsKeyPressed(KEY_BACKSPACE)) {
            shouldReturn = true; // Exit loop to return to menu
        }

        if ((searching || inserting) && !searchPath.empty()) {
            searchTimer += deltaTime;
            if (searchTimer >= 0.5f) {
                searchIndex++;
                searchTimer = 0.0f;
                if (searchIndex >= static_cast<int>(searchPath.size())) {
                    searchIndex = 0;
                    if (searching) {
                        if (!searchPath.empty() && searchPath.back()->key == lastSearchValue) {
                            searchResult = "Node " + std::to_string(lastSearchValue) + " is found";
                        }
                        else {
                            searchResult = "Node " + std::to_string(lastSearchValue) + " is not found";
                        }
                    }
                    searching = false;
                    inserting = false;
                }
            }
        }

        tree.updateAnimation(deltaTime);

        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("AVL Visualise", screenWidth / 2 - MeasureText("AVL Visualise", 100) / 2, screenHeight / 2 - 50, 100, Fade(GRAY, 0.2f));

        

        std::vector<Node*> currentHighlight;
        if (searching || inserting) {
            if (searchIndex < static_cast<int>(searchPath.size())) {
                currentHighlight.push_back(searchPath[searchIndex]);
            }
        }
        else if (undoClicked || redoClicked) {
            if (!affectedPath.empty()) {
                currentHighlight.push_back(affectedPath.back());
            }
        }
        tree.draw(currentHighlight);

        drawButton(insertButton, "Insert", GREEN, insertHover, insertClicked);
        drawButton(deleteButton, "Delete", RED, deleteHover, deleteClicked);
        drawButton(searchButton, "Search", BLUE, searchHover, searchClicked);
        drawButton(clearButton, "Clear", PURPLE, clearHover, clearClicked);
        drawButton(randomButton, "Random", ORANGE, randomHover, randomClicked);
        drawButton(undoButton, "Undo", GRAY, undoHover, undoClicked);
        drawButton(redoButton, "Redo", TEAL, redoHover, redoClicked);
        drawButton(FileButton, "File", Mediumblue, FileHover, FileClicked);
        drawButton(returnButton, "Return", YELLOW, returnHover, returnClicked);

        DrawRectangleRec(inputBox, LIGHTGRAY);
        DrawRectangleLinesEx(inputBox, 2, BLACK);
        DrawText(inputBuffer, inputBox.x + 5, inputBox.y + 10, 30, BLACK);
        DrawText("Enter number, then click action", inputBox.x, inputBox.y + 40, 20, DARKGRAY);
        DrawText(searchResult.c_str(), 20, screenHeight - 160, 20, DARKGRAY);

        EndDrawing();
    }
}