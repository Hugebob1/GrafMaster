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

void saveSubGraphs(GraphChunk* subgraphs, int numParts, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Nie mozna otworzyc pliku do zapisu");
        return;
    }

    for (int i = 0; i < numParts; i++) {
        fprintf(file, "#%d\n", i + 1);

        for (int j = 0; j < subgraphs[i]->totalVertices; j++) {
            Vertex v = subgraphs[i]->vertices[j];
            if (!v || v->degree == 0) continue;

            fprintf(file, "%d:", v->id);
            for (int k = 0; k < v->degree; k++) {
                fprintf(file, " %d", v->edges[k]);
            }
            fprintf(file, " (%d,%d)\n", v->x, v->y); 
        }
    }

    fclose(file);
    printf("Grafy zapisane do pliku: %s\n", filename);
}

void saveSubGraphsCompactBinary(GraphChunk* subgraphs, uint8_t numParts, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Nie mozna otworzyc pliku do zapisu");
        return;
    }

    fwrite(&numParts, sizeof(uint8_t), 1, file);

    for (uint8_t i = 0; i < numParts; i++) {
        GraphChunk g = subgraphs[i];

        uint16_t count = 0;
        for (int j = 0; j < g->totalVertices; j++) {
            if (g->vertices[j]) count++;
        }

        fwrite(&count, sizeof(uint16_t), 1, file);

        for (int j = 0; j < g->totalVertices; j++) {
            Vertex v = g->vertices[j];
            if (!v) continue;

            uint16_t id = (uint16_t)v->id;
            int16_t x = (int16_t)v->x;
            int16_t y = (int16_t)v->y;
            uint8_t deg = (uint8_t)v->degree;

            fwrite(&id, sizeof(uint16_t), 1, file);
            fwrite(&x, sizeof(int16_t), 1, file);
            fwrite(&y, sizeof(int16_t), 1, file);
            fwrite(&deg, sizeof(uint8_t), 1, file);

            for (int k = 0; k < deg; k++) {
                uint16_t neighbor = (uint16_t)v->edges[k];
                fwrite(&neighbor, sizeof(uint16_t), 1, file);
            }
        }
    }

    fclose(file);
    printf("Grafy zapisane do pliku binarnego: %s\n", filename);
}


GraphChunk* loadSubGraphsFromBinary(const char* filename, int* outNumParts) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie mozna otworzyc pliku binarnego");
        return NULL;
    }

    uint8_t numParts;
    fread(&numParts, sizeof(uint8_t), 1, file);
    *outNumParts = numParts;

    GraphChunk* parts = malloc(sizeof(GraphChunk) * numParts);

    for (int i = 0; i < numParts; i++) {
        uint16_t count;
        fread(&count, sizeof(uint16_t), 1, file);

        int maxId = -1;
        long start = ftell(file);

        for (int j = 0; j < count; j++) {
            uint16_t id;
            fread(&id, sizeof(uint16_t), 1, file);
            fseek(file, sizeof(int16_t) * 2, SEEK_CUR); // skip x, y
            uint8_t deg;
            fread(&deg, sizeof(uint8_t), 1, file);
            fseek(file, deg * sizeof(uint16_t), SEEK_CUR);

            if (id > maxId) maxId = id;
        }

        fseek(file, start, SEEK_SET);

        GraphChunk g = malloc(sizeof(struct GraphChunk));
        g->totalVertices = maxId + 1;
        g->vertices = calloc(g->totalVertices, sizeof(Vertex));

        for (int j = 0; j < count; j++) {
            uint16_t id;
            int16_t x, y;
            uint8_t deg;

            fread(&id, sizeof(uint16_t), 1, file);
            fread(&x, sizeof(int16_t), 1, file);
            fread(&y, sizeof(int16_t), 1, file);
            fread(&deg, sizeof(uint8_t), 1, file);

            Vertex v = createVertex(id, deg);
            v->x = x;
            v->y = y;
            v->degree = deg;

            for (int d = 0; d < deg; d++) {
                uint16_t neighbor;
                fread(&neighbor, sizeof(uint16_t), 1, file);
                v->edges[d] = neighbor;
            }

            g->vertices[id] = v;
        }

        parts[i] = g;
    }

    fclose(file);
    return parts;
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
        perror("Nie mozna otworzyc pliku binarnego");
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
        perror("Nie mozna otworzyc pliku binarnego do odczytu");
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

static int* degreeCmpHelper = NULL;

int compareDegreeDesc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return degreeCmpHelper[ib] - degreeCmpHelper[ia];
}

GraphChunk* splitGraphGreedyBalanced(GraphChunk graph, int numParts, float maxDiffPercent) {
    int total = graph->totalVertices;
    int* assignment = malloc(sizeof(int) * total);
    int* partSizes = calloc(numParts, sizeof(int));
    for (int i = 0; i < total; i++) assignment[i] = -1;

    int* degrees = malloc(sizeof(int) * total);
    int* order = malloc(sizeof(int) * total);
    for (int i = 0; i < total; i++) {
        order[i] = i;
        degrees[i] = (graph->vertices[i]) ? graph->vertices[i]->degree : -1;
    }
    degreeCmpHelper = degrees;
    qsort(order, total, sizeof(int), compareDegreeDesc);

    int* seeds = malloc(sizeof(int) * numParts);
    int seedCount = 0;
    for (int i = 0; i < total && seedCount < numParts; i++) {
        int v = order[i];
        if (graph->vertices[v]) {
            seeds[seedCount++] = v;
            assignment[v] = seedCount - 1;
            partSizes[seedCount - 1]++;
        }
    }

    bool* assigned = calloc(total, sizeof(bool));
    int* queues[numParts];
    int fronts[numParts], backs[numParts];
    for (int i = 0; i < numParts; i++) {
        queues[i] = malloc(sizeof(int) * total);
        fronts[i] = 0;
        backs[i] = 0;
        queues[i][backs[i]++] = seeds[i];
        assigned[seeds[i]] = true;
    }

    bool updated;
    do {
        updated = false;
        for (int g = 0; g < numParts; g++) {
            int count = backs[g];
            while (fronts[g] < count) {
                int u = queues[g][fronts[g]++];
                Vertex v = graph->vertices[u];
                if (!v) continue;
                for (int j = 0; j < v->degree; j++) {
                    int nei = v->edges[j];
                    if (!assigned[nei] && graph->vertices[nei]) {
                        assignment[nei] = g;
                        queues[g][backs[g]++] = nei;
                        partSizes[g]++;
                        assigned[nei] = true;
                        updated = true;
                    }
                }
            }
        }
    } while (updated);

    for (int i = 0; i < total; i++) {
        if (assignment[i] == -1 && graph->vertices[i]) {
            int minIdx = 0;
            for (int j = 1; j < numParts; j++) {
                if (partSizes[j] < partSizes[minIdx]) minIdx = j;
            }
            assignment[i] = minIdx;
            partSizes[minIdx]++;
        }
    }

    int minSize = total, maxSize = 0;
    for (int i = 0; i < numParts; i++) {
        if (partSizes[i] < minSize) minSize = partSizes[i];
        if (partSizes[i] > maxSize) maxSize = partSizes[i];
    }

    for (int i = 0; i < numParts; i++) {
        if (partSizes[i] == 0) {
            fprintf(stderr, "Blad: grupa %d jest pusta - nie mozna podzielic grafu na %d spojnych czesci\n", i, numParts);
            goto fail;
        }
    }

    int baseSize = total / numParts;
    float diff = (maxSize - minSize) / (float)baseSize * 100.0f;
    if (diff > maxDiffPercent) {
        printf("Blad: roznica wielkosci grup przekracza %.2f%% (aktualnie: %.2f%%)\n", maxDiffPercent, diff);
        goto fail;
    }

    GraphChunk* parts = malloc(sizeof(GraphChunk) * numParts);
    for (int i = 0; i < numParts; i++) {
        parts[i] = malloc(sizeof(struct GraphChunk));
        parts[i]->totalVertices = total;
        parts[i]->vertices = calloc(total, sizeof(Vertex));
    }

    for (int i = 0; i < total; i++) {
        int g = assignment[i];
        if (g == -1 || !graph->vertices[i]) continue;

        Vertex orig = graph->vertices[i];
        Vertex copy = createVertex(orig->id, orig->degree);
        copy->degree = 0;

        // 👇 KLUCZOWE: kopiowanie wspolrzednych x/y
        copy->x = orig->x;
        copy->y = orig->y;

        parts[g]->vertices[i] = copy;
    }

    for (int i = 0; i < total; i++) {
        int g = assignment[i];
        if (g == -1 || !graph->vertices[i]) continue;
        Vertex orig = graph->vertices[i];
        Vertex copy = parts[g]->vertices[i];
        for (int j = 0; j < orig->degree; j++) {
            int nei = orig->edges[j];
            if (assignment[nei] == g) {
                copy->edges[copy->degree++] = nei;
            }
        }
    }

    for (int i = 0; i < numParts; i++) {
        int validCount = 0;
        for (int j = 0; j < total; j++) {
            Vertex v = parts[i]->vertices[j];
            if (v && v->degree > 0) {
                validCount++;
                break;
            }
        }
        if (validCount == 0) {
            fprintf(stderr, "Nie udalo sie podzielic grafu\n");
            for (int k = 0; k < numParts; k++) {
                for (int j = 0; j < total; j++) free(parts[k]->vertices[j]);
                free(parts[k]->vertices);
                free(parts[k]);
            }
            free(parts);
            return NULL;
        }
    }

    free(assignment);
    free(partSizes);
    free(degrees);
    free(order);
    free(seeds);
    free(assigned);
    for (int i = 0; i < numParts; i++) free(queues[i]);

    return parts;

fail:
    free(assignment);
    free(partSizes);
    free(degrees);
    free(order);
    free(seeds);
    free(assigned);
    for (int i = 0; i < numParts; i++) free(queues[i]);
    return NULL;
}