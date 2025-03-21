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
    int maxwidth;          // Rozmiar macierzy XY (maxwidth x maxwidth)
    int numVertices;       // Liczba wierzchołków
    Node** adjLists;       // Listy sąsiedztwa
    int** xy;              // Macierz obecności wierzchołków (1 = jest wierzchołek)

    int* visited;          // Tablica odwiedzonych wierzchołków
    int** xyToVertex;      // Mapa z pozycji (i, j) -> indeks wierzchołka
    int* vertexToXY;       // Mapa z indeksu wierzchołka -> pozycja i * maxwidth + j

    int* componentId;      // componentId[i] = numer składowej spójnej, do której należy wierzchołek i
    int numComponents;     // Liczba składowych spójnych
} Graph;



// Function prototypes
Node* createNode(int v);
Graph* createGraph(int vertices, int maxwidth);
void addEdge(Graph* graph, int src, int dest);
void printGraph(Graph* graph);
Graph* loadGraph(FILE* file);

// Podział grafu na k części, z różnicą max X% w liczbie węzłów
void exportGraph(Graph* graph, const char* filename);

#endif
