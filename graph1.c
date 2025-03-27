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

bool validateGraphChunk(GraphChunk graph) {
    if (!graph || !graph->vertices) {
        printf("Blad: Graf jest NULL\n");
        return false;
    }

    bool valid = true;
    int totalEdges = 0;

    for (int i = 0; i < graph->totalVertices; i++) {
        Vertex v = graph->vertices[i];
        if (!v) {
            printf("Ostrzezenie: Brak wierzcholka o ID %d\n", i);
            valid = false;
            continue;
        }

        for (int j = 0; j < v->degree; j++) {
            int neighborId = v->edges[j];
            if (neighborId < 0 || neighborId >= graph->totalVertices || !graph->vertices[neighborId]) {
                printf("Blad: Wierzcholek %d ma niepoprawnego sasiada %d\n", v->id, neighborId);
                valid = false;
                continue;
            }

            // Sprawdzenie odwrotnej krawędzi
            Vertex neighbor = graph->vertices[neighborId];
            bool foundReverse = false;
            for (int k = 0; k < neighbor->degree; k++) {
                if (neighbor->edges[k] == v->id) {
                    foundReverse = true;
                    break;
                }
            }
            if (!foundReverse) {
                printf("Uwaga: brak odwrotnej krawedzi (%d -> %d)\n", neighborId, v->id);
                // Nie uznajemy tego jako "błąd krytyczny", tylko ostrzeżenie
            }

            totalEdges++;
        }
    }
    printf("Graf wczytano poprawnie\n");
    printf("Sprawdzono %d wierzcholkow, lacznie %d krawedzi (nieskierowanych: %d)\n",
           graph->totalVertices, totalEdges, totalEdges / 2);

    return valid;
}

void dfs(Vertex* vertices, int currentId, bool* visited, int totalVertices) {
    visited[currentId] = true;
    Vertex v = vertices[currentId];

    for (int i = 0; i < v->degree; i++) {
        int neighborId = v->edges[i];
        if (!visited[neighborId]) {
            dfs(vertices, neighborId, visited, totalVertices);
        }
    }
}

bool isGraphConnected(GraphChunk graph) {
    if (!graph || !graph->vertices) {
        printf("Blad: graf jest NULL\n");
        return false;
    }

    bool* visited = calloc(graph->totalVertices, sizeof(bool));
    if (!visited) {
        perror("calloc failed");
        exit(2137);
    }

    // Szukamy pierwszego niepustego wierzchołka
    int start = -1;
    for (int i = 0; i < graph->totalVertices; i++) {
        if (graph->vertices[i] && graph->vertices[i]->degree > 0) {
            start = i;
            break;
        }
    }

    if (start == -1) {
        printf("Brak aktywnych wierzcholkoww – graf pusty\n");
        free(visited);
        return false;
    }

    dfs(graph->vertices, start, visited, graph->totalVertices);

    // Sprawdzamy czy wszystkie niepuste wierzchołki zostały odwiedzone
    bool connected = true;
    for (int i = 0; i < graph->totalVertices; i++) {
        if (graph->vertices[i] && graph->vertices[i]->degree > 0 && !visited[i]) {
            printf("Wierzcholek %d nie zostal osiągniety – graf niespojny\n", i);
            connected = false;
        }
    }
    if(connected) printf("Graf jest spojny\n");
    free(visited);
    return connected;
}

void saveGraphBinaryCompact(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Nie można otworzyć pliku binarnego");
        return;
    }
    int totalVertices = graph->totalVertices;
    fwrite(&totalVertices, sizeof(int), 1, file); // <<--- nagłówek
    for (int i=0;i<totalVertices;i++) {
        Vertex v = graph->vertices[i];
        uint8_t degree = (uint8_t) v->degree;
        fwrite(&degree, sizeof(uint8_t), 1, file);

        for (int j = 0; j < degree; j++) {
            uint16_t neighbor = (uint16_t) v->edges[j];
            fwrite(&neighbor, sizeof(uint16_t), 1, file);
        }
    }
    printf("Graf zapisany do pliku binarnego: %s\n", filename);
    fclose(file);
}

GraphChunk loadGraphFromBinaryToChunk(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie można otworzyć pliku binarnego do odczytu");
        return NULL;
    }

    int numVertices = 0;
    fread(&numVertices, sizeof(int), 1, file);

    GraphChunk graph = malloc(sizeof(struct GraphChunk));
    graph->totalVertices = numVertices;
    graph->vertices = malloc(sizeof(Vertex) * numVertices);

    for (int i = 0; i < numVertices; i++) {
        uint8_t degree;
        fread(&degree, sizeof(uint8_t), 1, file);

        Vertex vertex = createVertex(i, degree);
        vertex->degree = degree;

        for (int j = 0; j < degree; j++) {
            uint16_t neighbor;
            fread(&neighbor, sizeof(uint16_t), 1, file);
            vertex->edges[j] = neighbor;
        }

        graph->vertices[i] = vertex;
    }

    fclose(file);
    return graph;
}
