#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Structure for adjacency list node

typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

// Graph structure
typedef struct Graph {
    int maxwidth;
    int numVertices;
    Node** adjLists;
    int** xy;
} Graph;



// Function prototypes
Node* createNode(int v);
Graph* createGraph(int vertices);
void addEdge(Graph* graph, int src, int dest);
void printGraph(Graph* graph);
Graph* loadGraph(FILE* file);

// Podział grafu na k części, z różnicą max X% w liczbie węzłów
void exportGraph(Graph* graph, const char* filename);

#endif
