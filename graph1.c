#include "graph1.h"
#include "utils.h"
#include "validation.h"

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
    // v->x = -1; // initial value
    // v->y = -1; // initial value
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
        graph->vertices[i] = createVertex(i, 0);  // przypisujemy wskaźnik
    }

    return graph;
}

GraphChunk addEdges(const char *fileName, int x) {
    FILE *in = fopen(fileName, "r");
    int vertecies;
    fscanf(in, "%d", &vertecies);
    fclose(in);
    const int n2 = numElements(fileName, 2); // liczba wierzchołków
    const int n3 = numElements(fileName, 3); // liczba czegos tam
    const int numConnections = numElements(fileName, 4);
    const int lines = numLines(fileName);
    int graphNumber = 1;
    
    if (x > 0) {
        if (x <= lines - 4)
            graphNumber = x;
        else {
            fprintf(stderr, "Nie ma takiego grafu w pliku: %s\n", fileName);
            return NULL;
        }
    } else if (lines > 5) {
        fprintf(stderr, "W pliku występuje więcej niż jeden graf. Grafy do wyboru: ");
        for (int i = 0; i < lines - 4; i++) {
            fprintf(stderr, "%d ", i + 1);
        }
        fprintf(stderr, "\nProsze uruchomic program z odpowiednia flaga -g\n");
        return NULL;
    }

    const int numSections = numElements(fileName, graphNumber + 4);
    int *connections = readLine(fileName, 4, numConnections);
    int *sections = readLine(fileName, graphNumber + 4, numSections);
    int *line2 = readLine(fileName, 2, n2);
    int *line3 = readLine(fileName, 3, n3);
    validatefile(vertecies, line2, n2, line3, n3, connections, numConnections, sections, numSections);
    if (!connections || !sections) {
        fprintf(stderr, "Blad: nie udalo się wczytac polaczen lub sekcji z pliku %s\n", fileName);
        return NULL;
    }

    GraphChunk graph = malloc(sizeof(struct GraphChunk));
    graph->totalVertices = n2;
    graph->vertices = malloc(sizeof(Vertex) * n2);

    // Alokacja i inicjalizacja wierzchołków
    for (int i = 0; i < n2; i++) {
        graph->vertices[i] = createVertex(i, 20); // zapas miejsca
        graph->vertices[i]->degree = 0;
    }

    // Dodawanie krawędzi (sekcje główne)
    for (int i = 0; i < numSections - 1; i++) {
        int a = sections[i], b = sections[i + 1];
        int pom = a, pom1 = pom + 1;
        while (a < b - 1) {
            int u = connections[pom];
            int v = connections[pom1];
            if (u != v) {
                Vertex vu = graph->vertices[u];
                Vertex vv = graph->vertices[v];

                if (vu->degree >= vu->numEdges) {
                    vu->numEdges *= 2;
                    vu->edges = realloc(vu->edges, vu->numEdges * sizeof(int));
                    if (!vu->edges) {
                        fprintf(stderr, "Blad realloc edges dla wierzcholka %d\n", vu->id);
                        exit(EXIT_FAILURE);
                    }
                }
                vu->edges[vu->degree++] = v;
                
                if (vv->degree >= vv->numEdges) {
                    vv->numEdges *= 2;
                    vv->edges = realloc(vv->edges, vv->numEdges * sizeof(int));
                    if (!vv->edges) {
                        fprintf(stderr, "Blad realloc edges dla wierzcholka %d\n", vv->id);
                        exit(EXIT_FAILURE);
                    }
                }
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

            if (vu->degree >= vu->numEdges) {
                vu->numEdges *= 2;
                vu->edges = realloc(vu->edges, vu->numEdges * sizeof(int));
                if (!vu->edges) {
                    fprintf(stderr, "Blad realloc edges dla wierzcholka %d\n", vu->id);
                    exit(EXIT_FAILURE);
                }
            }
            vu->edges[vu->degree++] = v;
            
            if (vv->degree >= vv->numEdges) {
                vv->numEdges *= 2;
                vv->edges = realloc(vv->edges, vv->numEdges * sizeof(int));
                if (!vv->edges) {
                    fprintf(stderr, "Blad realloc edges dla wierzcholka %d\n", vv->id);
                    exit(EXIT_FAILURE);
                }
            }
            vv->edges[vv->degree++] = u;
        }
        a++;
        pom1++;
    }

    int global_y = 0;
    int pomIndex = 0;
    if (n3 >= 2 && line3[0] == 0 && line3[1] == 0) {
        global_y = 1;
    }
    for (int i = 0; i < n3 - 1; i++) {
        int left = line3[i], right = line3[i + 1];
        if (left == 0 && right == 0) {
            continue;
        }

        while (left < right) {
            graph->vertices[pomIndex]->x = line2[left];
            graph->vertices[pomIndex]->y = global_y;
            left++;
            pomIndex++;
        }

        global_y += 1;
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
        printf(" (%d,%d)\n", v->x, v->y);
    }
}

void freeGraphChunk(GraphChunk graph) {
    if (!graph) return;
    if (graph->vertices) {
        for (int i = 0; i < graph->totalVertices; i++) {
            if (graph->vertices[i]) {
                free(graph->vertices[i]->edges);
                free(graph->vertices[i]);
            }
        }
        free(graph->vertices);
    }
    free(graph);
}




