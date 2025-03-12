#include <stdio.h>
#include <stdlib.h>

// Struktura reprezentująca węzeł listy sąsiedztwa
typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

// Struktura reprezentująca graf
typedef struct Graph {
    int numVertices;
    Node** adjLists;
} Graph;

// Funkcja tworząca nowy węzeł listy sąsiedztwa
Node* createNode(int v) {
    Node* newNode = malloc(sizeof(Node));
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

void loadGraph(FILE* file) {
    int maxWidth;
    fscanf(file, "%d", &maxWidth);  // Wczytanie maksymalnej liczby węzłów w wierszu
    printf("Maksymalna liczba wezlow w wierszu: %d\n", maxWidth);
    int x = 0;
    int table[maxWidth][maxWidth];
    for(int i=0;i<maxWidth;i++){
        for(int j=0;j<maxWidth;j++){
            table[i][j]=0;
        }
    }
    int i=0, pom=0;
    while(fscanf(file, "%d;", &x)==1){
        table[i][x] = 1;
        if(i>0){
            if(x<pom){
                i++;
                pom = x;
            }
        }
        pom = x;
    }
    for(int i=0;i<maxWidth;i++){
        for(int j=0;j<maxWidth;j++){
            printf("%d ", table[i][j]);
        }
        printf("\n");
    }
}

// Funkcja tworząca graf
Graph* createGraph(int vertices) {
    Graph* graph = malloc(sizeof(Graph));
    graph->numVertices = vertices;
    graph->adjLists = malloc(vertices * sizeof(Node*));

    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL;
    }
    return graph;
}

// Funkcja dodająca krawędź do grafu
void addEdge(Graph* graph, int src, int dest) {
    // Dodanie krawędzi z src do dest
    Node* newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;

    // Ponieważ graf jest nieskierowany, dodajemy także odwrotną krawędź
    newNode = createNode(src);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

// Funkcja wyświetlająca graf
void printGraph(Graph* graph) {
    for (int i = 0; i < graph->numVertices; i++) {
        Node* temp = graph->adjLists[i];
        printf("Wierzcholek %d:\n", i);
        while (temp) {
            printf(" -> %d", temp->vertex);
            temp = temp->next;
        }
        printf("\n");
    }
}

// Główna funkcja
int main(int argc, char **argv) {
    FILE *in = argc > 1 ? fopen(argv[1], "r"): stdin;
    int vertices = 5; // Przykładowa liczba wierzchołków
    Graph* graph = createGraph(vertices);
    //Graph* test = loadGraph(in);
    loadGraph(in);
    addEdge(graph, 0, 1);
    addEdge(graph, 0, 4);
    addEdge(graph, 1, 2);
    addEdge(graph, 1, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 3);
    addEdge(graph, 3, 4);

    printGraph(graph);

    return 0;
}