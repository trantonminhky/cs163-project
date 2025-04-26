#include "raylib.h"
#include "AVL.h"
#include "Common.h"
#include <vector>
#include <string>
#include <iostream>
#include <random>

Node::Node(int value) : key(value), height(1), left(nullptr), right(nullptr), x(0), y(0), targetX(0), targetY(0), isDying(false) {}

AVLTree::AVLTree() : root(nullptr), instantMode(false), currentOperation("") {
    instantBtn = { 900, 880, 100, 40 };
    insertCode = {
        "insert(node, key):",
        "  if node is null:",
        "    return new Node(key)",
        "  if key < node.key:",
        "    node.left = insert(node.left, key)",
        "  else if key > node.key:",
        "    node.right = insert(node.right, key)",
        "  else:",
        "    return node // Duplicate key",
        "  updateHeight(node)",
        "  balance = getBalance(node)",
        "  if balance > 1 and key < node.left.key:",
        "    return rightRotate(node)",
        "  if balance < -1 and key > node.right.key:",
        "    return leftRotate(node)",
        "  if balance > 1 and key > node.left.key:",
        "    node.left = leftRotate(node.left)",
        "    return rightRotate(node)",
        "  if balance < -1 and key < node.right.key:",
        "    node.right = rightRotate(node.right)",
        "    return leftRotate(node)",
        "  return node"
    };
    searchCode = {
        "search(node, key):",
        "  if node is null:",
        "    return not found",
        "  if key == node.key:",
        "    return found",
        "  if key < node.key:",
        "    return search(node.left, key)",
        "  else:",
        "    return search(node.right, key)"
    };
}

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

Node* AVLTree::insert(Node* node, int key, std::vector<Node*>& path, std::string& searchResult, std::vector<int>& codePath) {
    codePath.push_back(0); // Line: insert(node, key)
    if (!node) {
        codePath.push_back(2); // Line: return new Node(key)
        Node* newNode = new Node(key);
        path.push_back(newNode);
        return newNode;
    }

    path.push_back(node);
    codePath.push_back(3); // Line: if key < node.key
    if (key < node->key) {
        codePath.push_back(4); // Line: node.left = insert(node.left, key)
        node->left = insert(node->left, key, path, searchResult, codePath);
    }
    else {
        codePath.push_back(5); // Line: else if key > node.key
        if (key > node->key) {
            codePath.push_back(6); // Line: node.right = insert(node.right, key)
            node->right = insert(node->right, key, path, searchResult, codePath);
        }
        else {
            codePath.push_back(7); // Line: return node // Duplicate key
            searchResult = "The value of node is already in tree";
            return node;
        }
    }

    codePath.push_back(9); // Line: updateHeight(node)
    updateHeight(node);
    codePath.push_back(10); // Line: balance = getBalance(node)
    int balance = getBalance(node);

    codePath.push_back(11); // Line: if balance > 1 and key < node.left.key
    if (balance > 1 && key < node->left->key) {
        codePath.push_back(12); // Line: return rightRotate(node)
        return rightRotate(node);
    }
    codePath.push_back(13); // Line: if balance < -1 and key > node.right.key
    if (balance < -1 && key > node->right->key) {
        codePath.push_back(14); // Line: return leftRotate(node)
        return leftRotate(node);
    }
    codePath.push_back(15); // Line: if balance > 1 and key > node.left.key
    if (balance > 1 && key > node->left->key) {
        codePath.push_back(16); // Line: node.left = leftRotate(node.left)
        node->left = leftRotate(node->left);
        codePath.push_back(17); // Line: return rightRotate(node)
        return rightRotate(node);
    }
    codePath.push_back(18); // Line: if balance < -1 and key < node.right.key
    if (balance < -1 && key < node->right->key) {
        codePath.push_back(19); // Line: node.right = rightRotate(node.right)
        node->right = rightRotate(node->right);
        codePath.push_back(20); // Line: return leftRotate(node)
        return leftRotate(node);
    }

    codePath.push_back(21); // Line: return node
    return node;
}

void AVLTree::insert(int key, std::string& searchResult) {
    Node* treeCopy = deepCopy(root);
    std::vector<Node*> path;
    std::vector<int> codePath;
    currentOperation = "insert";
    currentCodePath.clear();
    searchResult = "";
    root = insert(root, key, path, searchResult, codePath);
    if (searchResult.empty()) {
        history.push({ treeCopy, {true, key} });
        while (!redoStack.empty()) {
            deleteTree(redoStack.top().first);
            redoStack.pop();
        }
        calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
    }
    else {
        deleteTree(treeCopy);
    }
    currentCodePath = codePath;
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
    search(value, affectedPath, currentCodePath);
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
    search(value, affectedPath, currentCodePath);
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

void AVLTree::search(int key, std::vector<Node*>& searchPath, std::vector<int>& codePath) {
    searchPath.clear();
    codePath.clear();
    Node* current = root;
    int step = 0;
    while (current && step < 100) {
        searchPath.push_back(current);
        codePath.push_back(0); // Line: search(node, key)
        codePath.push_back(3); // Line: if key == node.key
        if (key == current->key) {
            codePath.push_back(4); // Line: return found
            break;
        }
        codePath.push_back(5); // Line: if key < node.key
        if (key < current->key) {
            codePath.push_back(6); // Line: return search(node.left, key)
            current = current->left;
        }
        else {
            codePath.push_back(7); // Line: return search(node.right, key)
            current = current->right;
        }
        codePath.push_back(1); // Line: if node is null
        if (!current) {
            codePath.push_back(2); // Line: return not found
            break;
        }
        step++;
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
        if (instantMode) {
            node->x = node->targetX;
            node->y = node->targetY;
        }
        else {
            float dx = node->targetX - node->x;
            float dy = node->targetY - node->y;
            node->x += dx * deltaTime * 0.5f;
            node->y += dy * deltaTime * 0.5f;
        }
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
        std::string dummyResult;
        insert(randomKey, dummyResult);
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
    if (instantMode) {
        while (fscanf_s(file, "%d", &value) == 1) {
            std::string dummyResult;
            insert(value, dummyResult);
            calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
            updateAnimation(0.0f);
            count++;
        }
        searchResult = "Instantly loaded " + std::to_string(count) + " values from " + std::string(filePath);
    }
    else {
        std::vector<Node*> path;
        while (fscanf_s(file, "%d", &value) == 1) {
            path.clear();
            std::string dummyResult;
            root = insert(root, value, path, dummyResult, currentCodePath);
            calculatePositions(root, GetScreenWidth() / 2, 50, 300, 1);
            count++;
        }
        searchResult = "Loaded " + std::to_string(count) + " values from " + std::string(filePath);
    }

    fclose(file);
}
void AVLTree::DrawCodeBox(int screenWidth, int screenHeight, int currentCodeIndex) {
    static float codeBoxAlpha = 0.0f;
    static float codeBoxY = static_cast<float>(screenHeight);
    static bool codeBoxVisible = false;

    if (!currentOperation.empty()) { // Changed tree.currentOperation to currentOperation
        codeBoxVisible = true;

        const std::vector<std::string>& code = (currentOperation == "insert") ? insertCode : searchCode; // Changed tree.insertCode and tree.searchCode
        int maxLines = std::min(static_cast<int>(code.size()), 20);
        int boxWidth = 270;
        int lineHeight = 18;
        int padding = 10;
        int boxHeight = maxLines * lineHeight + 2 * padding;

        int xPos = screenWidth - boxWidth - 30;
        float targetY = static_cast<float>(screenHeight - boxHeight - 30);

        // Animate fade-in
        codeBoxAlpha += 0.05f;
        if (codeBoxAlpha > 1.0f) codeBoxAlpha = 1.0f;

        // Animate slide-in
        if (codeBoxY > targetY) {
            codeBoxY -= 10.0f;
            if (codeBoxY < targetY) codeBoxY = targetY;
        }

        // Draw background
        DrawRectangle(xPos, static_cast<int>(codeBoxY), boxWidth, boxHeight, Fade(LIGHTGRAY, 0.8f * codeBoxAlpha));

        // Draw code lines
        int yOffset = static_cast<int>(codeBoxY) + padding;
        for (int i = 0; i < maxLines; ++i) {
            Color textColor = (i == currentCodeIndex) ? RED : BLACK;
            std::string displayLine = code[i].substr(0, 35); // Limit line width
            DrawText(displayLine.c_str(), xPos + 10, yOffset, 15, Fade(textColor, codeBoxAlpha));
            yOffset += lineHeight;
        }
    }
    else {
        // Reset for next time
        codeBoxVisible = false;
        codeBoxAlpha = 0.0f;
        codeBoxY = static_cast<float>(screenHeight);
    }
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
    float operationTimer = 0.0f;
    int operationIndex = 0;
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
    Color instantColor = { 255, 105, 180, 255 };

    bool shouldReturn = false;

    while (!WindowShouldClose() && !shouldReturn) {
        float deltaTime = GetFrameTime();

        // Input handling: Accept printable characters
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32 && key <= 126) && inputIndex < 9) { // Printable ASCII characters
                inputBuffer[inputIndex++] = (char)key;
                inputBuffer[inputIndex] = '\0';
            }
            key = GetCharPressed();
        }
        // Handle backspace for input box
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
        bool instantHover = CheckCollisionPointRec(GetMousePosition(), tree.instantBtn);
        bool returnHover = CheckCollisionPointRec(GetMousePosition(), returnButton);

        bool insertClicked = isButtonClicked(insertButton) || (IsKeyPressed(KEY_ENTER) && inputIndex > 0);
        bool deleteClicked = isButtonClicked(deleteButton);
        bool searchClicked = isButtonClicked(searchButton);
        bool clearClicked = isButtonClicked(clearButton);
        bool randomClicked = isButtonClicked(randomButton);
        bool undoClicked = isButtonClicked(undoButton);
        bool redoClicked = isButtonClicked(redoButton);
        bool FileClicked = isButtonClicked(FileButton);
        bool instantClicked = isButtonClicked(tree.instantBtn);
        bool returnClicked = isButtonClicked(returnButton);

        if (instantClicked) {
            tree.instantMode = !tree.instantMode;
            searchResult = tree.instantMode ? "Instant Mode ON" : "Step-by-Step Mode ON";
            if (tree.instantMode) {
                searching = false;
                inserting = false;
                searchPath.clear();
                insertPath.clear();
                operationTimer = 0.0f;
                operationIndex = 0;
                tree.currentOperation = "";
                tree.currentCodePath.clear();
            }
        }

        if (insertClicked && inputIndex > 0) {
            try {
                int value = std::stoi(inputBuffer);
                insertPath.clear();
                searchResult = "";
                tree.insert(value, searchResult);
                if (searchResult.empty()) {
                    inputIndex = 0;
                    inputBuffer[0] = '\0';
                    if (!tree.instantMode) {
                        tree.currentOperation = "insert";
                        tree.search(value, insertPath, tree.currentCodePath);
                        searchPath = insertPath;
                        operationIndex = 0;
                        operationTimer = 0.0f;
                        inserting = true;
                    }
                    else {
                        searchPath.clear();
                        inserting = false;
                        tree.updateAnimation(0.0f);
                        tree.currentOperation = "";
                        tree.currentCodePath.clear();
                    }
                }
                else {
                    // Clear input on duplicate key
                    inputIndex = 0;
                    inputBuffer[0] = '\0';
                }
            }
            catch (const std::exception& e) {
                searchResult = "Invalid input. Please enter a number.";
                tree.currentOperation = "";
                tree.currentCodePath.clear();
                inputIndex = 0;
                inputBuffer[0] = '\0';
            }
        }
        if (deleteClicked && inputIndex > 0) {
            try {
                int value = std::stoi(inputBuffer);
                tree.deleteNode(value);
                inputIndex = 0;
                inputBuffer[0] = '\0';
                searching = false;
                inserting = false;
                searchPath.clear();
                insertPath.clear();
                searchResult = "";
                tree.currentOperation = "";
                tree.currentCodePath.clear();
                if (tree.instantMode) {
                    tree.updateAnimation(0.0f);
                }
            }
            catch (const std::exception& e) {
                searchResult = "Invalid input. Please enter a number.";
                tree.currentOperation = "";
                tree.currentCodePath.clear();
                inputIndex = 0;
                inputBuffer[0] = '\0';
            }
        }
        if (searchClicked && inputIndex > 0) {
            try {
                lastSearchValue = std::stoi(inputBuffer);
                tree.currentOperation = "search";
                tree.currentCodePath.clear();
                tree.search(lastSearchValue, searchPath, tree.currentCodePath);
                inputIndex = 0;
                inputBuffer[0] = '\0';
                if (!tree.instantMode) {
                    operationIndex = 0;
                    operationTimer = 0.0f;
                    searching = true;
                    searchResult = "Searching for " + std::to_string(lastSearchValue) + "...";
                }
                else {
                    if (!searchPath.empty() && searchPath.back()->key == lastSearchValue) {
                        searchResult = "Node " + std::to_string(lastSearchValue) + " is found";
                    }
                    else {
                        searchResult = "Node " + std::to_string(lastSearchValue) + " is not found";
                    }
                    searching = false;
                    searchPath.clear();
                    tree.currentOperation = "";
                    tree.currentCodePath.clear();
                }
            }
            catch (const std::exception& e) {
                searchResult = "Invalid input. Please enter a number.";
                tree.currentOperation = "";
                tree.currentCodePath.clear();
                inputIndex = 0;
                inputBuffer[0] = '\0';
            }
        }
        if (clearClicked) {
            tree.clear();
            searchPath.clear();
            insertPath.clear();
            affectedPath.clear();
            searching = false;
            inserting = false;
            searchResult = "";
            tree.currentOperation = "";
            tree.currentCodePath.clear();
            inputIndex = 0;
            inputBuffer[0] = '\0';
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
            tree.currentOperation = "";
            tree.currentCodePath.clear();
            inputIndex = 0;
            inputBuffer[0] = '\0';
            if (tree.instantMode) {
                tree.updateAnimation(0.0f);
            }
        }
        if (undoClicked) {
            Node* affectedNode = tree.undo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
            tree.currentOperation = "";
            tree.currentCodePath.clear();
            inputIndex = 0;
            inputBuffer[0] = '\0';
            if (tree.instantMode && affectedNode) {
                tree.updateAnimation(0.0f);
            }
        }
        if (redoClicked) {
            Node* affectedNode = tree.redo(affectedPath);
            searching = false;
            inserting = false;
            searchPath.clear();
            insertPath.clear();
            searchResult = "";
            tree.currentOperation = "";
            tree.currentCodePath.clear();
            inputIndex = 0;
            inputBuffer[0] = '\0';
            if (tree.instantMode && affectedNode) {
                tree.updateAnimation(0.0f);
            }
        }
        if (FileClicked) {
            tree.LoadFromFile(searchResult);
            searchPath.clear();
            inserting = false;
            searching = false;
            tree.currentOperation = "";
            tree.currentCodePath.clear();
            inputIndex = 0;
            inputBuffer[0] = '\0';
        }
        // Only exit on backspace if input box is empty
        if (returnClicked || (IsKeyPressed(KEY_BACKSPACE) && inputIndex == 0)) {
            shouldReturn = true;
        }

        if ((searching || inserting) && !searchPath.empty() && !tree.instantMode) {
            operationTimer += deltaTime;
            if (operationTimer >= 0.5f) {
                operationIndex++;
                operationTimer = 0.0f;
                if (operationIndex >= static_cast<int>(searchPath.size())) {
                    operationIndex = 0;
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
                    tree.currentOperation = "";
                    tree.currentCodePath.clear();
                    inputIndex = 0;
                    inputBuffer[0] = '\0';
                }
            }
        }

        tree.updateAnimation(deltaTime);

        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("AVL Visualise", screenWidth / 2 - MeasureText("AVL Visualise", 100) / 2, screenHeight / 2 - 50, 100, Fade(GRAY, 0.2f));

        std::vector<Node*> currentHighlight;
        int currentCodeIndex = -1;
        if ((searching || inserting) && !tree.instantMode && operationIndex < static_cast<int>(searchPath.size())) {
            currentHighlight.push_back(searchPath[operationIndex]);
            int codePathIndex = operationIndex * (tree.currentCodePath.size() / std::max(1, static_cast<int>(searchPath.size())));
            if (codePathIndex < static_cast<int>(tree.currentCodePath.size())) {
                currentCodeIndex = tree.currentCodePath[codePathIndex];
            }
        }

        tree.draw(currentHighlight);

        tree.DrawCodeBox(screenWidth, screenHeight, currentCodeIndex);

        drawButton(insertButton, "Insert", GREEN, insertHover, insertClicked);
        drawButton(deleteButton, "Delete", RED, deleteHover, deleteClicked);
        drawButton(searchButton, "Search", BLUE, searchHover, searchClicked);
        drawButton(clearButton, "Clear", PURPLE, clearHover, clearClicked);
        drawButton(randomButton, "Random", ORANGE, randomHover, randomClicked);
        drawButton(undoButton, "Undo", GRAY, undoHover, undoClicked);
        drawButton(redoButton, "Redo", TEAL, redoHover, redoClicked);
        drawButton(FileButton, "File", Mediumblue, FileHover, FileClicked);
        drawButton(tree.instantBtn, tree.instantMode ? "Instant" : "Step", instantColor, instantHover, instantClicked);
        drawButton(returnButton, "Return", YELLOW, returnHover, returnClicked);

        // Add visual feedback for invalid input
        bool isValidInput = true;
        for (int i = 0; i < inputIndex; ++i) {
            if (inputBuffer[i] < '0' || inputBuffer[i] > '9') {
                isValidInput = false;
                break;
            }
        }
        Color inputBorderColor = (!isValidInput || searchResult.find("Invalid") != std::string::npos) ? RED : BLACK;
        DrawRectangleRec(inputBox, LIGHTGRAY);
        DrawRectangleLinesEx(inputBox, 2, inputBorderColor);
        DrawText(inputBuffer, inputBox.x + 5, inputBox.y + 10, 30, BLACK);
        DrawText("Enter number, then click action or press Enter", inputBox.x, inputBox.y + 40, 20, DARKGRAY);
        DrawText(searchResult.c_str(), 20, screenHeight - 160, 20, DARKGRAY);

        EndDrawing();
    }
}