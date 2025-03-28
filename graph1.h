#ifndef GRAPH1_H
#define GRAPH1_H

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

typedef struct Vertex {
    int id;
    int numEdges;
    int halfEdges;
    int* edges;
    int* internalEdges;
    int* externalEdges;
    int groupId;
    int edgeDelta;
    int degree;
    int active;
} *Vertex;

typedef struct GraphChunk {
    Vertex* vertices;       // tablica wierzchołków
    int totalVertices;      // rozmiar grafu
} *GraphChunk;

//struktura pliku binarnego
// [Vertecies (unit32_t)]
// [degree (uint8_t)] 
// [neighbor_1 (uint16_t)]
// [neighbor_2 (uint16_t)]
// ...
// [neighbor_degree (uint16_t)]

// Function prototypes
Vertex createVertex(int id, int numEdges); //ok
GraphChunk createGraphChunk(const char *fileName); //ok
GraphChunk addEdges(const char *fileName); //ok
void printGraphChunk(GraphChunk graph); //ok
void freeGraphChunk(GraphChunk graph); //ok
void exportGraph(GraphChunk graph, const char* filename); //ok

//jutro reszta
bool validateGraphChunk(GraphChunk graph); //ok
void dfs(Vertex* vertices, int currentId, bool* visited, int totalVertices); //ok
bool isGraphConnected(GraphChunk graph); //ok
void saveGraphBinaryCompact(GraphChunk graph, const char* filename); //ok
GraphChunk loadGraphFromBinaryToChunk(const char* filename); //ok
int compareDegreeDesc(const void* a, const void* b);
GraphChunk* splitGraphGreedyBalanced(GraphChunk graph, int numParts, float maxDiffPercent);

#endif
