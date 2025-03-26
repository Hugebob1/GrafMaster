#include "graph.h"
#include "utils.h"

#define max_line 65536

// Create a new node for adjacency list
Vertex createVertex(int id, int numEdges) {
    Vertex v = malloc(sizeof(struct Vertex));
    if (!v) {
        perror("Memory allocation failed!");
        exit(2137);
    }
    v->id = id;
    v->numEdges = numEdges;
    v->halfEdges = numEdges / 2;
    v->edges = malloc(sizeof(int) * v->numEdges);
    v->internalEdges = malloc(sizeof(int) * v->halfEdges);
    v->externalEdges = malloc(sizeof(int) * v->halfEdges);

    v->groupId = -1;
    v->edgeDelta = 0;
    v->degree = 0;
    v->active = 1;

    return v;
}


int findLastNode(const char *fileName) {
    const int n5 = numElements(fileName, 5);
    int *line5 = readLine(fileName, 5, n5);

    int lastIndex = line5[n5 - 1];

    free(line5);

    return n5; 
}


GraphChunk createGraphChunk(const char *fileName) {
    GraphChunk first = NULL;
    GraphChunk temp = NULL;
    GraphChunk current = NULL;
    GraphChunk prev = NULL;
    const int lastNode = findLastNode(fileName);
    for (int i = 0; i < lastNode; i++) {
        temp = malloc(sizeof(struct GraphChunk));
        if (!temp) {
            printf("Memory allocation failed!");
            exit(2137);
        }
        temp->next = NULL;
        temp->totalVertices = lastNode;
        temp->vertex = NULL;  // jeszcze nie przypisujemy wierzchołka

        if (i == 0) {
            first = temp;
        }
        if (prev == NULL) {
            current = temp;
        } else {
            prev->next = temp;
        }
        prev = temp;
    }
    return first;
}


/*Creating a graph*/
GraphChunk addEdges(const char *fileName) {
    const int n2 = numElements(fileName, 2); // liczba wierzchołków
    const int numConnections = numElements(fileName, 4);
    const int lines = numLines(fileName);
    int graphNumber = 1;
    if(lines>5){
        printf("liczba grafow w pliku: %d\n", lines-4);
        printf("Ktory graf mam wczytac? (1 - %d)\n", lines-4);
        scanf("%d", &graphNumber);
    }
    const int numSections = numElements(fileName, graphNumber+4);

    int *connections = readLine(fileName, 4, numConnections);
    int *sections = readLine(fileName, graphNumber+4, numSections);

    // 🔧 Przygotuj tablicę GraphChunk dla każdego wierzchołka 0..n2-1
    GraphChunk* idMap = calloc(n2, sizeof(GraphChunk));
    GraphChunk graph = NULL;
    GraphChunk last = NULL;

    for (int i = 0; i < n2; i++) {
        GraphChunk chunk = malloc(sizeof(struct GraphChunk));
        chunk->next = NULL;
        chunk->totalVertices = n2;
        chunk->vertex = createVertex(i, 20); // zapas miejsca
        chunk->vertex->degree = 0;

        if (!graph) graph = chunk;
        else last->next = chunk;
        last = chunk;

        idMap[i] = chunk;
    }

    // 🔁 Główna logika dodawania krawędzi (dokładnie jak w Twoim działającym kodzie)
    for (int i = 0; i < numSections - 1; i++) {
        int a = sections[i], b = sections[i + 1];
        int pom = a, pom1 = pom + 1;

        while (a < b - 1) {
            int u = connections[pom];
            int v = connections[pom1];
            if (u != v) {
                addUndirectedEdge(idMap, u, v);
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
            addUndirectedEdge(idMap, u, v);
        }
        a++;
        pom1++;
    }

    free(connections);
    free(sections);
    GraphChunk result = idMap[0];
    free(idMap);
    return result;
}

void addUndirectedEdge(GraphChunk* idMap, int u, int v) {
    GraphChunk a = idMap[u];
    GraphChunk b = idMap[v];

    // Dodaj v do sąsiadów u
    bool found = false;
    for (int i = 0; i < a->vertex->degree; i++) {
        if (a->vertex->edges[i] == v) {
            found = true;
            break;
        }
    }
    if (!found && a->vertex->degree < a->vertex->numEdges) {
        a->vertex->edges[a->vertex->degree++] = v;
    }

    // Dodaj u do sąsiadów v
    found = false;
    for (int i = 0; i < b->vertex->degree; i++) {
        if (b->vertex->edges[i] == u) {
            found = true;
            break;
        }
    }
    if (!found && b->vertex->degree < b->vertex->numEdges) {
        b->vertex->edges[b->vertex->degree++] = u;
    }
}




void makeGraphUndirected(GraphChunk graph) {
    for (GraphChunk current = graph; current != NULL; current = current->next) {
        int from = current->vertex->id;

        for (int i = 0; i < current->vertex->degree; i++) {
            int to = current->vertex->edges[i];

            // Znajdź "to"
            for (GraphChunk rev = graph; rev != NULL; rev = rev->next) {
                if (rev->vertex && rev->vertex->id == to) {
                    // Sprawdź, czy from już istnieje w jego sąsiadach
                    bool found = false;
                    for (int j = 0; j < rev->vertex->degree; j++) {
                        if (rev->vertex->edges[j] == from) {
                            found = true;
                            break;
                        }
                    }

                    if (!found && rev->vertex->degree < rev->vertex->numEdges) {
                        rev->vertex->edges[rev->vertex->degree++] = from;
                    }
                    break;
                }
            }
        }
    }
}



void printGraphChunk(GraphChunk graph) {
    GraphChunk current = graph;

    while (current) {
        if (!current->vertex) {
            current = current->next;
            continue;
        }

        printf("Wierzcholek %d:\n", current->vertex->id);
        for (int i = 0; i < current->vertex->degree; i++) {
            printf(" -> %d", current->vertex->edges[i]);
        }
        printf("\n");

        current = current->next;
    }
}


void freeGraphChunk(GraphChunk graph) {
    GraphChunk current = graph;
    while (current) {
        GraphChunk next = current->next;
        free(current->vertex->edges);
        free(current->vertex->internalEdges);
        free(current->vertex->externalEdges);
        free(current->vertex);
        free(current);
        current = next;
    }
}

#include <stdbool.h>

bool validateGraphChunk(GraphChunk graph) {
    if (!graph) {
        printf("Błąd: graf jest NULL\n");
        return false;
    }

    bool valid = true;
    int edgeCount = 0;
    int vertexCount = 0;

    // Sprawdzenie połączeń
    for (GraphChunk current = graph; current != NULL; current = current->next) {
        if (!current->vertex) {
            printf("Uwaga: pusty wierzcholek w GraphChunk\n");
            continue;
        }

        int id = current->vertex->id;
        vertexCount++;

        for (int i = 0; i < current->vertex->degree; i++) {
            int neighbor = current->vertex->edges[i];

            if (neighbor < 0) {
                printf("Błąd: wierzcholek %d ma niepoprawnego sasiada: %d\n", id, neighbor);
                valid = false;
                continue;
            }

            // Szukamy odwrotnej krawędzi
            bool found = false;
            for (GraphChunk check = graph; check != NULL; check = check->next) {
                if (check->vertex && check->vertex->id == neighbor) {
                    for (int j = 0; j < check->vertex->degree; j++) {
                        if (check->vertex->edges[j] == id) {
                            found = true;
                            break;
                        }
                    }
                    break;
                }
            }

            if (!found) {
                printf("Uwaga: brak odwrotnej krawedzi (%d -> %d)\n", neighbor, id);
                // Możesz zmienić to na błąd:
                // valid = false;
            }

            edgeCount++;
        }
    }

    printf("Sprawdzono %d wierzcholkow, lacznie %d krawedzi (nieskierowanych: %d)\n",
           vertexCount, edgeCount, edgeCount / 2);

    return valid;
}

void exportGraph(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Nie można otworzyć pliku do zapisu");
        return;
    }

    GraphChunk current = graph;

    while (current) {
        if (!current->vertex) {
            current = current->next;
            continue;
        }
        fprintf(file, "%d: ", current->vertex->id);
        for (int i = 0; i < current->vertex->degree; i++) {
            if(i==current->vertex->degree-1)
                fprintf(file, "%d\n", current->vertex->edges[i]);
            else
                fprintf(file, "%d ", current->vertex->edges[i]);
        }
        current = current->next;
    }

    fclose(file);
    printf("Graf zapisany do pliku: %s\n", filename);
}

void saveGraphBinaryCompact(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Nie można otworzyć pliku binarnego");
        return;
    }

    int totalVertices = graph->totalVertices;
    fwrite(&totalVertices, sizeof(int), 1, file); // <<--- nagłówek

    for (GraphChunk curr = graph; curr != NULL; curr = curr->next) {
        uint8_t degree = (uint8_t) curr->vertex->degree;
        fwrite(&degree, sizeof(uint8_t), 1, file);

        for (int i = 0; i < degree; i++) {
            uint16_t neighbor = (uint16_t) curr->vertex->edges[i];
            fwrite(&neighbor, sizeof(uint16_t), 1, file);
        }
    }

    fclose(file);
}

GraphChunk loadGraphFromBinaryToChunk(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie można otworzyć pliku binarnego do odczytu");
        return NULL;
    }

    // 📥 Odczytaj liczbę wierzchołków z nagłówka
    int numVertices = 0;
    fread(&numVertices, sizeof(int), 1, file);

    GraphChunk head = NULL;
    GraphChunk last = NULL;

    for (int nodeId = 0; nodeId < numVertices; nodeId++) {
        uint8_t degree;
        fread(&degree, sizeof(uint8_t), 1, file);

        Vertex vertex = createVertex(nodeId, degree);
        vertex->degree = degree;

        for (int i = 0; i < degree; i++) {
            uint16_t neighbor;
            fread(&neighbor, sizeof(uint16_t), 1, file);
            vertex->edges[i] = neighbor;
        }

        GraphChunk chunk = malloc(sizeof(struct GraphChunk));
        chunk->vertex = vertex;
        chunk->totalVertices = numVertices;
        chunk->next = NULL;

        if (!head) {
            head = chunk;
        } else {
            last->next = chunk;
        }
        last = chunk;
    }

    fclose(file);
    return head;
}


int countVertices(GraphChunk graph) {
    int count = 0;
    for (GraphChunk curr = graph; curr != NULL; curr = curr->next)
        count++;
    return count;
}

void bfsAssign(GraphChunk graph, int* visited, int* assignment, int currentGroup, int targetSize, int totalVertices) {
    int queue[totalVertices];
    int front = 0, back = 0;

    // Znajdź pierwszy nieodwiedzony wierzchołek
    GraphChunk start = graph;
    while (start && visited[start->vertex->id])
        start = start->next;

    if (!start) return;

    int assigned = 0;
    queue[back++] = start->vertex->id;
    visited[start->vertex->id] = 1;
    assignment[start->vertex->id] = currentGroup;
    assigned++;

    while (front < back && assigned < targetSize) {
        int u = queue[front++];
        GraphChunk uChunk = graph;
        while (uChunk && uChunk->vertex->id != u)
            uChunk = uChunk->next;

        if (!uChunk) continue;

        for (int i = 0; i < uChunk->vertex->degree; i++) {
            int v = uChunk->vertex->edges[i];
            if (!visited[v]) {
                queue[back++] = v;
                visited[v] = 1;
                assignment[v] = currentGroup;
                assigned++;
                if (assigned >= targetSize) break;
            }
        }
    }
}

GraphChunk extractSubgraph(GraphChunk original, int* assignment, int group, int totalVertices) {
    GraphChunk head = NULL, last = NULL;

    for (GraphChunk curr = original; curr != NULL; curr = curr->next) {
        int id = curr->vertex->id;
        if (assignment[id] != group) continue;

        // policz sąsiadów należących do tej samej grupy
        int count = 0;
        for (int i = 0; i < curr->vertex->degree; i++) {
            int neighbor = curr->vertex->edges[i];
            if (assignment[neighbor] == group)
                count++;
        }

        Vertex v = createVertex(id, count);
        v->degree = count;
        int k = 0;
        for (int i = 0; i < curr->vertex->degree; i++) {
            int neighbor = curr->vertex->edges[i];
            if (assignment[neighbor] == group)
                v->edges[k++] = neighbor;
        }

        GraphChunk chunk = malloc(sizeof(struct GraphChunk));
        chunk->vertex = v;
        chunk->totalVertices = totalVertices;
        chunk->next = NULL;

        if (!head) head = chunk;
        else last->next = chunk;

        last = chunk;
    }

    return head;
}

GraphChunk* splitGraphBalancedConnected(GraphChunk graph, int numParts, float maxDiffPercent) {
    int totalVertices = countVertices(graph);
    if (numParts <= 0 || totalVertices == 0) {
        printf("Nieprawidlowa liczba czesci lub pusty graf.\n");
        return NULL;
    }

    int* visited = calloc(totalVertices, sizeof(int));
    int* assignment = malloc(sizeof(int) * totalVertices);
    memset(assignment, -1, sizeof(int) * totalVertices);

    int targetSize = totalVertices / numParts;
    int remaining = totalVertices;

    for (int i = 0; i < numParts; i++) {
        int size = (i == numParts - 1) ? remaining : targetSize;
        bfsAssign(graph, visited, assignment, i, size, totalVertices);
        remaining -= size;
    }

    free(visited);

    // Zbuduj podgrafy
    GraphChunk* parts = malloc(sizeof(GraphChunk) * numParts);
    int sizes[numParts];
    int minSize = totalVertices, maxSize = 0;

    for (int i = 0; i < numParts; i++) {
        parts[i] = extractSubgraph(graph, assignment, i, totalVertices);

        int count = countVertices(parts[i]);
        sizes[i] = count;
        if (count < minSize) minSize = count;
        if (count > maxSize) maxSize = count;
    }

    float avg = (float)totalVertices / numParts;
    float diff = maxSize - minSize;
    float diffPercent = (diff / avg) * 100.0f;

    if (diffPercent > maxDiffPercent) {
        printf("❌ Nie da sie podzielic grafu na %d części z roznica≤ %.2f%% (rzeczywista: %.2f%%)\n",
               numParts, maxDiffPercent, diffPercent);
        free(assignment);
        for (int i = 0; i < numParts; i++) {
            // zwalnianie pamięci jeśli chcesz
        }
        free(parts);
        return NULL;
    }

    printf("✅ Podzielono graf na %d części (max roznica: %.2f%%)\n", numParts, diffPercent);
    free(assignment);
    return parts;
}

