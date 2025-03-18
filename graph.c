#include "graph.h"
#include "utils.h"

// Create a new node for adjacency list
Node* createNode(int v) {
    Node* newNode = malloc(sizeof(Node));
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

// Create a new graph
Graph* createGraph(int vertices) {
    Graph* graph = malloc(sizeof(Graph));
    graph->numVertices = vertices;
    graph->adjLists = malloc(vertices * sizeof(Node*));

    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL;
    }
    return graph;
}

// Add edge to the graph
void addEdge(Graph* graph, int src, int dest) {
    Node* newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;

    newNode = createNode(src);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

// Load graph from file
Graph* loadGraph(FILE* file) {
    int maxWidth;
    fscanf(file, "%d\n", &maxWidth);
    printf("Maksymalna liczba wezlow w wierszu: %d\n", maxWidth);

    int vertices = 0, numtokens = 0;
    char line[3000];
    char line4[8000];

    if (fgets(line, sizeof(line), file) != NULL) {
        vertices = how_many_digits(line);
    }

    Graph* graph = createGraph(vertices);

    if (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';
    }
    if (fgets(line4, sizeof(line4), file) != NULL) {
        line4[strcspn(line4, "\n")] = '\0';
        numtokens = how_many_digits(line4);
        int connections[numtokens];
        read_digits(line4, connections);

        if (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            int sections_cnt = how_many_digits(line);
            if (sections_cnt >= 2) {
                int sections[sections_cnt];
                read_digits(line, sections);
                for (int i = 0; i < sections_cnt - 1; i++) {
                    int a = sections[i], b = sections[i + 1];
                    int pom = a, pom1 = pom + 1;
                    while (a < b - 1) {
                        addEdge(graph, connections[pom], connections[pom1]);
                        a++;
                        pom1++;
                    }
                }
                int a = sections[sections_cnt - 1];
                int b = numtokens;
                int pom = a, pom1 = pom + 1;
                while (a < b - 1) {
                    addEdge(graph, connections[pom], connections[pom1]);
                    a++;
                    pom1++;
                }
            }
        }
    }

    fclose(file);
    return graph;
}

// Print graph adjacency list
void printGraph(Graph* graph) {
    for (int i = 0; i < graph->numVertices; i++) {
        Node* temp = graph->adjLists[i];
        if(temp == NULL) {
            continue;
        }
        printf("Wierzcholek %d:\n", i);
        while (temp) {
            printf(" -> %d", temp->vertex);
            temp = temp->next;
        }
        printf("\n");
    }
}

void exportGraph(Graph* graph, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Nie można otworzyć pliku %s\n", filename);
        return;
    }

    for (int i = 0; i < graph->numVertices; i++) {
        Node* temp = graph->adjLists[i];
        while (temp) {
            if (i < temp->vertex) { // Unikamy duplikowania krawędzi w nieskierowanym grafie
                fprintf(file, "%d,%d\n", i, temp->vertex);
            }
            temp = temp->next;
        }
    }
    
    fclose(file);
    printf("Graf zapisano do %s\n", filename);
}
