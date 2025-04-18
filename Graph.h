#ifndef GRAPH_H
#define GRAPH_H

#include "raylib.h"
#include <vector>
#include <string>
#include "tinyfiledialogs.h"

using namespace std;

struct Vertex
{
    Vector2 position;
    int id;
    bool selected;
    float scale;
    float alpha;
    Vector2 targetPos;
};

struct Edge
{
    int from;
    int to;
    int weight;
    bool highlighted;
    bool blurred;
};

struct GraphState
{
    vector<Vertex> vertices;
    vector<Edge> edges;
};

class GraphApp
{
private:
    vector<Vertex> vertices;
    vector<Edge> edges;
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
    string searchResult;
    bool showSearchMessage;
    float kruskalTimer;
    int kruskalStep;
    vector<Edge> mstEdges;
    bool showTable;
    int totalWeight;
    vector<GraphState> undoStack;
    vector<GraphState> redoStack;
    vector<int> parent;
    bool instantMode; // For instant execution toggle
    Rectangle instantBtn; // Instant mode button

    void HandleInitialize();
    void GenerateRandomGraph();
    bool WouldCollide(int excludeIndex, Vector2 newPos);
    Vector2 FindVacantPosition();
    bool EdgeExists(int fromIndex, int toIndex);
    void ProcessInput();
    void ProcessInsert();
    void ProcessDelete();
    void ProcessFileInput();
    int GetVertexAt(Vector2 pos);
    bool VertexExists(int id);
    int GetVertexIndex(int id);
    void ResetGraphState();
    void MakeSet(int v);
    int Find(int v);
    void Union(int a, int b);
    void RunKruskalStepByStep();

public:
    GraphApp();
    void Run();
    void Update();
    void Draw(bool& shouldReturn);
};

#endif