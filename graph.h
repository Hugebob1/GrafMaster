#ifndef GRAPH_H
#define GRAPH_H

//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#define LINE_BUF 65536
//struktura pliku binarnego

/*FILE graf.csrrg
 * Linijki 1-3 potrzebne do macierzy
 * Linijki 2-5 potrzebne do grafu
 */


 typedef struct Vertex {
    int id;                   // Unikalny ID wierzchołka
    int numEdges;             // Liczba wszystkich krawędzi (rozmiar tablicy edges)
    int halfEdges;            // Połowa rozmiaru: numEdges / 2

    int* edges;               // Wszystkie sąsiadujące wierzchołki
    int* internalEdges;       // Sąsiedzi z tej samej grupy
    int* externalEdges;       // Sąsiedzi z innych grup

    int groupId;              // Przynależność do grupy/spójnej składowej/klastra
    int edgeDelta;            // Różnica: zewnętrzne - wewnętrzne

    int degree;               // Stopień wierzchołka (opcjonalnie == numEdges)
    int active;               // Flaga aktywności (np. dla algorytmu Louvain)
} *Vertex;



typedef struct GraphChunk {
    Vertex vertex;              // Wskaźnik na wierzchołek
    int totalVertices;          // Liczba wszystkich wierzchołków w tym kawałku

    struct GraphChunk* next;    // Kolejny fragment (np. dla listy fragmentów grafu)
} *GraphChunk;


//struktura pliku binarnego
// [degree (uint8_t)] 
// [neighbor_1 (uint16_t)]
// [neighbor_2 (uint16_t)]
// ...
// [neighbor_degree (uint16_t)]





// Function prototypes
Vertex createVertex(int id, int numEdges);
GraphChunk createGraphChunk(const char *fileName);
GraphChunk addEdges(const char *fileName);
int findLastNode(const char *fileName);
void printGraphChunk(GraphChunk graph);
void freeGraphChunk(GraphChunk graph);
void addUndirectedEdge(GraphChunk* idMap, int u, int v);
bool validateGraphChunk(GraphChunk graph);
void exportGraph(GraphChunk graph, const char* filename);
void saveGraphBinaryCompact(GraphChunk graph, const char* filename);
GraphChunk loadGraphFromBinaryToChunk(const char* filename);
int countVertices(GraphChunk graph);
void bfsAssign(GraphChunk graph, int* visited, int* assignment, int currentGroup, int targetSize, int totalVertices);
GraphChunk extractSubgraph(GraphChunk original, int* assignment, int group, int totalVertices);
GraphChunk* splitGraphBalancedConnected(GraphChunk graph, int numParts, float maxDiffPercent);



// Node* createNode(int v);
// Graph* createGraph(int vertices, int maxwidth);
// void addEdge(Graph* graph, int src, int dest);
// void printGraph(Graph* graph);
// Graph* loadGraph(FILE* file);
// void freeGraph(Graph *graph);
// // Podział grafu na k części, z różnicą max X% w liczbie węzłów
// void exportGraph(Graph* graph, const char* filename);
// void saveGraphBinary(Graph* graph, const char* filename);
// Graph* loadGraphBinary(const char* filename);

#endif
