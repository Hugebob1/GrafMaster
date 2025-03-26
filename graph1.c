#include "graph1.h"
#include "utils.h"

Vertex createVertex(int id, int numEdges) {
    Vertex v = malloc(sizeof(struct Vertex));
    if (!v) {
        perror("Błąd alokacji pamięci dla wierzchołka");
        exit(EXIT_FAILURE);
    }

    v->id = id;
    v->numEdges = numEdges;
    v->halfEdges = numEdges / 2;
    v->edges = malloc(sizeof(int) * numEdges);
    v->internalEdges = malloc(sizeof(int) * v->halfEdges);
    v->externalEdges = malloc(sizeof(int) * v->halfEdges);
    v->groupId = -1;
    v->edgeDelta = 0;
    v->degree = numEdges;   // jeśli chcesz osobno sterować – możesz zmienić
    v->active = 1;

    return v;
}

GraphChunk createGraphChunk(const char *fileName) {
    GraphChunk graph = malloc(sizeof(struct GraphChunk));
    if (!graph) {
        perror("Błąd alokacji GraphChunk");
        exit(EXIT_FAILURE);
    }

    // Załóżmy, że liczba wierzchołków to linia 2 w pliku
    FILE *file = fopen(fileName, "r");
    if (!file) {
        perror("Błąd otwierania pliku");
        exit(EXIT_FAILURE);
    }

    int numVertices = 0;
    char line[512];
    int currentLine = 1;

    // Szukamy linii 2 – czyli tej z unikalnymi ID wierzchołków
    while (fgets(line, sizeof(line), file)) {
        if (currentLine == 2) {
            char* ptr = line;
            int value;
            while (sscanf(ptr, "%d", &value) == 1) {
                numVertices++;
                while (*ptr == ' ' || (*ptr >= '0' && *ptr <= '9')) ptr++;
            }
            break;
        }
        currentLine++;
    }

    fclose(file);

    // Alokacja tablicy wierzchołków
    graph->totalVertices = numVertices;
    graph->vertices = malloc(sizeof(struct Vertex) * numVertices);
    if (!graph->vertices) {
        perror("Błąd alokacji tablicy wierzchołków");
        exit(EXIT_FAILURE);
    }

    // Wypełniamy domyślnie puste wierzchołki (ID = indeks)
    for (int i = 0; i < numVertices; i++) {
        for (int i = 0; i < numVertices; i++) {
            graph->vertices[i] = createVertex(i, 0);  // przypisujemy wskaźnik
        } 
    }

    return graph;
}

GraphChunk addEdges(const char *fileName) {
    const int n2 = numElements(fileName, 2); // liczba wierzchołków
    const int numConnections = numElements(fileName, 4);
    const int lines = numLines(fileName);
    int graphNumber = 1;
    if (lines > 5) {
        printf("liczba grafow w pliku: %d\n", lines - 4);
        printf("Ktory graf mam wczytac? (1 - %d)\n", lines - 4);
        scanf("%d", &graphNumber);
    }
    const int numSections = numElements(fileName, graphNumber + 4);

    int *connections = readLine(fileName, 4, numConnections);
    int *sections = readLine(fileName, graphNumber + 4, numSections);

    GraphChunk graph = malloc(sizeof(struct GraphChunk));
    graph->totalVertices = n2;
    graph->vertices = malloc(sizeof(Vertex) * n2);

    // Alokacja i inicjalizacja wierzchołków
    for (int i = 0; i < n2; i++) {
        graph->vertices[i] = createVertex(i, 20); // zapas miejsca
        graph->vertices[i]->degree = 0;
    }

    // Dodawanie krawędzi
    for (int i = 0; i < numSections - 1; i++) {
        int a = sections[i], b = sections[i + 1];
        int pom = a, pom1 = pom + 1;
        while (a < b - 1) {
            int u = connections[pom];
            int v = connections[pom1];
            if (u != v) {
                Vertex vu = graph->vertices[u];
                Vertex vv = graph->vertices[v];
                vu->edges[vu->degree++] = v;
                vv->edges[vv->degree++] = u;
            }
            a++;
            pom1++;
        }
    }

    // Ostatnia sekcja
    int a = sections[numSections - 1];
    int b = numConnections;
    int pom = a, pom1 = pom + 1;
    while (a < b - 1) {
        int u = connections[pom];
        int v = connections[pom1];
        if (u != v) {
            Vertex vu = graph->vertices[u];
            Vertex vv = graph->vertices[v];
            vu->edges[vu->degree++] = v;
            vv->edges[vv->degree++] = u;
        }
        a++;
        pom1++;
    }

    free(connections);
    free(sections);
    return graph;
}

void printGraphChunk(GraphChunk graph) {
    for (int i = 0; i < graph->totalVertices; i++) {
        Vertex v = graph->vertices[i];
        if (!v || v->degree == 0) continue;

        printf("Wierzcholek %d:\n", v->id);
        for (int j = 0; j < v->degree; j++) {
            printf(" -> %d", v->edges[j]);
        }
        printf("\n");
    }
}

void freeGraphChunk(GraphChunk graph) {
    for (int i = 0; i < graph->totalVertices; i++) {
        free(graph->vertices[i]->edges);
        free(graph->vertices[i]->internalEdges);
        free(graph->vertices[i]->externalEdges);
        free(graph->vertices[i]);
    }
    free(graph->vertices);
    free(graph);
}

void exportGraph(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Nie można otworzyć pliku do zapisu");
        return;
    }

    for (int i = 0; i < graph->totalVertices; i++) {
        Vertex v = graph->vertices[i];
        if (!v || v->degree == 0) continue;

        fprintf(file, "%d: ", v->id);
        for (int j = 0; j < v->degree; j++) {
            if(j==v->degree-1)
                fprintf(file, "%d\n", v->edges[j]);
            else
                fprintf(file, "%d ", v->edges[j]);
        }
    }

    fclose(file);
    printf("Graf zapisany do pliku: %s\n", filename);
}

