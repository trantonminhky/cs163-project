#ifndef GRAPH_H
#define GRAPH_H

#include "raylib.h"
#include <vector>
#include <string>

// Vertex structure
struct Vertex
{
    Vector2 position;
    int id;
    bool selected;
    float scale;
    float alpha;
    Vector2 targetPos;
};

// Edge structure
struct Edge
{
    int from;
    int to;
    int weight;
    bool highlighted;
    bool blurred;
};

// Graph state for undo/redo
struct GraphState
{
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
};

class GraphApp
{
private:
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    char inputText[32];
    bool showInputBox;
    int screenWidth;
    int screenHeight;
    enum State
    {
        NORMAL,
        DRAGGING,
        RANDOM,
        SEARCH,
        KRUSKAL,
        INSERT,
        DELETE
    };
    State currentState;
    int draggedVertex;
    std::string searchResult;
    bool showSearchMessage;
    float kruskalTimer;
    int kruskalStep;
    std::vector<Edge> mstEdges;
    bool showTable;
    int totalWeight;
    std::vector<GraphState> undoStack;
    std::vector<GraphState> redoStack;
    std::vector<int> parent;

public:
    GraphApp();
    void Run();

private:
    void Update();
    void Draw();
    void HandleInitialize();
    void GenerateRandomGraph();
    bool WouldCollide(int excludeIndex, Vector2 newPos);
    Vector2 FindVacantPosition();
    bool EdgeExists(int fromIndex, int toIndex);
    void ProcessInput();
    void ProcessInsert();
    void ProcessDelete();
    int GetVertexAt(Vector2 pos);
    bool VertexExists(int id);
    int GetVertexIndex(int id);
    void ResetGraphState();
    void MakeSet(int v);
    int Find(int v);
    void Union(int a, int b);
    void RunKruskalStepByStep();
};

#endif