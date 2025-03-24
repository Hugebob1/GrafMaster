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
    const int numSections = numElements(fileName, 5);

    int *connections = readLine(fileName, 4, numConnections);
    int *sections = readLine(fileName, 5, numSections);

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

void saveGraphUniqueUndirected(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Nie można otworzyć pliku do zapisu");
        return;
    }

    for (GraphChunk curr = graph; curr != NULL; curr = curr->next) {
        int u = curr->vertex->id;

        for (int i = 0; i < curr->vertex->degree; i++) {
            int v = curr->vertex->edges[i];

            if (u < v) {
                fprintf(file, "%d - %d\n", u, v);
            }
        }
    }

    fclose(file);
    printf("Graf zapisany do pliku (unikalne krawędzie nieskierowane): %s\n", filename);
}




// void exportGraph(Graph* graph, const char* filename) {
//     FILE* file = fopen(filename, "w");
//     if (!file) {
//         printf("Nie można otworzyc pliku %s\n", filename);
//         return;
//     }

//     for (int i = 0; i < graph->numVertices; i++) {
//         Node* temp = graph->adjLists[i];
//         while (temp) {
//             if (i < temp->vertex) { // Unikamy duplikowania krawędzi w nieskierowanym grafie
//                 fprintf(file, "%d,%d\n", i, temp->vertex);
//             }
//             temp = temp->next;
//         }
//     }
    
//     fclose(file);
//     printf("Graf zapisano do %s\n", filename);
// }

// void saveGraphBinary(Graph* graph, const char* filename) {
//     FILE* file = fopen(filename, "wb");
//     if (!file) {
//         perror("Nie mozna otworzyc pliku do zapisu");
//         return;
//     }

//     uint32_t numVertices = graph->numVertices;
//     uint32_t edgeCount = 0;

//     // Najpierw policz liczbę krawędzi
//     for (int i = 0; i < numVertices; i++) {
//         for (Node* n = graph->adjLists[i]; n != NULL; n = n->next) {
//             if (i < n->vertex) edgeCount++;  // unikamy duplikatów w grafie nieskierowanym
//         }
//     }

//     fwrite(&numVertices, sizeof(uint32_t), 1, file);
//     fwrite(&edgeCount, sizeof(uint32_t), 1, file);

//     // Teraz zapisz krawędzie
//     for (int i = 0; i < numVertices; i++) {
//         for (Node* n = graph->adjLists[i]; n != NULL; n = n->next) {
//             if (i < n->vertex) {
//                 Edge e = { (uint32_t)i, (uint32_t)n->vertex };
//                 fwrite(&e, sizeof(Edge), 1, file);
//             }
//         }
//     }

//     fclose(file);
//     printf("Zapisano graf do %s (%u wierzcholkow, %u krawedzi)\n", filename, numVertices, edgeCount);
// }

// Graph* loadGraphBinary(const char* filename) {
//     FILE* file = fopen(filename, "rb");
//     if (!file) {
//         perror("Nie mozna otworzyć pliku do odczytu");
//         return NULL;
//     }

//     uint32_t numVertices, numEdges;

//     if (fread(&numVertices, sizeof(uint32_t), 1, file) != 1 ||
//         fread(&numEdges, sizeof(uint32_t), 1, file) != 1) {
//         fprintf(stderr, "Blad: nie udalo sie wczytac naglowka pliku.\n");
//         fclose(file);
//         return NULL;
//     }

//     // Stwórz pusty graf
//     Graph* graph = createGraph(numVertices, 0); // 0 = maxWidth nieistotne tutaj
//     if (!graph) {
//         fclose(file);
//         return NULL;
//     }

//     for (uint32_t i = 0; i < numEdges; i++) {
//         uint32_t from, to;
//         if (fread(&from, sizeof(uint32_t), 1, file) != 1 ||
//             fread(&to, sizeof(uint32_t), 1, file) != 1) {
//             fprintf(stderr, "Blad: nie udalo sie wczytac krawedzi %u.\n", i);
//             freeGraph(graph);
//             fclose(file);
//             return NULL;
//         }
//         addEdge(graph, from, to); // zakładamy, że addEdge dodaje też odwrotnie jeśli graf nieskierowany
//     }

//     fclose(file);
//     printf("Wczytano graf z pliku %s (%u wierzcholkow, %u krawedzi)\n", filename, numVertices, numEdges);
//     return graph;
// }

