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
    int x;
    int y;
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
extern int lastgrapherror;
extern int edgesErrors;
Vertex createVertex(int id, int numEdges); //ok
GraphChunk createGraphChunk(const char *fileName); //ok
GraphChunk addEdges(const char *fileName, int x); //ok
void printGraphChunk(GraphChunk graph); //ok
void freeGraphChunk(GraphChunk graph); //ok

#endif
