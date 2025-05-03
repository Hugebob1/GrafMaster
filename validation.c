#include "validation.h"

#define ERR_CHUNK_NULL               40  // graf lub jego tablica jest NULL
#define ERR_CHUNK_INVALID_NEIGHBOR  41  // błędny sąsiad: poza zakresem lub NULL
#define WARN_CHUNK_MISSING_VERTEX   42  // ostrzeżenie: brakujący wierzchołek
#define WARN_CHUNK_MISSING_BACKEDGE 43  // ostrzeżenie: brak odwrotnej krawędzi
#define CHUNK_VALID                 0   // wszystko OK


int validateGraphChunk(GraphChunk graph) {
    if (!graph || !graph->vertices) {
        printf("Blad: Graf jest NULL\n");
        return ERR_CHUNK_NULL;
    }

    int status = CHUNK_VALID;
    int totalEdges = 0;

    for (int i = 0; i < graph->totalVertices; i++) {
        Vertex v = graph->vertices[i];
        if (!v) {
            printf("Ostrzezenie: Brak wierzcholka o ID %d\n", i);
            return WARN_CHUNK_MISSING_VERTEX;
            break;
        }

        for (int j = 0; j < v->degree; j++) {
            int neighborId = v->edges[j];
            if (neighborId < 0 || neighborId >= graph->totalVertices || !graph->vertices[neighborId]) {
                printf("Blad: Wierzcholek %d ma niepoprawnego sasiada %d\n", v->id, neighborId);
                return ERR_CHUNK_INVALID_NEIGHBOR;
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
                if (status == CHUNK_VALID) status = WARN_CHUNK_MISSING_BACKEDGE;
            }

            totalEdges++;
        }
    }

    printf("Graf wczytano poprawnie\n");
    printf("Sprawdzono %d wierzcholkow, lacznie %d krawedzi (nieskierowanych: %d)\n",
           graph->totalVertices, totalEdges, totalEdges / 2);

    return status;
}

void dfs(Vertex* vertices, int currentId, bool* visited, int totalVertices) {
    visited[currentId] = true;
    Vertex v = vertices[currentId];

    for (int i = 0; i < v->degree; i++) {
        int neighborId = v->edges[i];

        // Bezpieczne sprawdzenie indeksu sąsiada
        if (neighborId < 0 || neighborId >= totalVertices) continue;
        if (!vertices[neighborId]) continue;

        if (!visited[neighborId]) {
            dfs(vertices, neighborId, visited, totalVertices);
        }
    }
}

// Kody błędów dla isGraphConnected
#define ERR_GRAPH_NULL              30
#define ERR_ALLOC_FAILED            31
#define ERR_NO_ACTIVE_VERTICES      32
#define ERR_GRAPH_NOT_CONNECTED     33
#define GRAPH_CONNECTED              0

int isGraphConnected(GraphChunk graph) {
    if (!graph || !graph->vertices) {
        // Graf nieistniejący lub niepoprawny
        return ERR_GRAPH_NULL;
    }

    int totalVertices = graph->totalVertices;
    bool* visited = calloc(totalVertices, sizeof(bool));
    if (!visited) {
        perror("calloc failed");
        return ERR_ALLOC_FAILED;
    }

    // Znajdź pierwszy aktywny wierzchołek
    int start = -1;
    for (int i = 0; i < totalVertices; i++) {
        if (graph->vertices[i] && graph->vertices[i]->degree > 0) {
            start = i;
            break;
        }
    }

    if (start == -1) {
        free(visited);
        return ERR_NO_ACTIVE_VERTICES;
    }

    dfs(graph->vertices, start, visited, totalVertices);

    // Sprawdź, czy wszystkie aktywne wierzchołki zostały odwiedzone
    for (int i = 0; i < totalVertices; i++) {
        if (graph->vertices[i] && graph->vertices[i]->degree > 0 && !visited[i]) {
            free(visited);
            return ERR_GRAPH_NOT_CONNECTED;
        }
    }

    free(visited);
    return GRAPH_CONNECTED;
}

#define ERR_FILE_TOO_FEW_LINES       20
#define ERR_FILE_NOT_ENOUGH_DATA     21
#define ERR_FILE_INVALID_INTERVAL    22
#define ERR_FILE_INTERVAL_MISMATCH   23
#define ERR_FILE_CONNECTION_MISMATCH 24
#define VALIDATION_SUCCESS            0

int validatefile(int vertecies, int* line2, int n2, int* line3, int n3, int* connectios, int numc, int* sections, int nums) {

    // 1. Sprawdzenie minimalnej liczby linii i niepustych linii
    if (vertecies <= 0 || n2 <= 0 || n3 <= 0 || numc <= 0 || nums <= 0) {
        fprintf(stderr, "Blad: plik zawiera puste linie lub za malo danych\n");
        return ERR_FILE_NOT_ENOUGH_DATA;
    }

    int nonEmptyLines = 0;
    if (vertecies > 0) nonEmptyLines++;
    if (n2 > 0) nonEmptyLines++;
    if (n3 > 0) nonEmptyLines++;
    if (numc > 0) nonEmptyLines++;
    if (nums > 0) nonEmptyLines++;

    if (nonEmptyLines < 5) {
        fprintf(stderr, "Blad: plik musi zawierac minimum 5 niepustych linii\n");
        return ERR_FILE_TOO_FEW_LINES;
    }

    int cnt = 0;

    // 2. Sprawdzenie poprawności line3 (czy przedziały rosnące)
    for (int i = 0; i < n3 - 1; i++) {
        if (line3[i + 1] - line3[i] >= 0) {
            cnt++;
        } else {
            fprintf(stderr, "Blad przedzialu w pliku wejsciowym\n");
            return ERR_FILE_INVALID_INTERVAL;
        }
    }

    if (cnt != vertecies) {
        fprintf(stderr, "Braki w przedzialach w pliku wejsciowym\n");
        return ERR_FILE_INTERVAL_MISMATCH;
    }

    // 3. Sprawdzenie, czy suma przedzialow pasuje do liczby polaczen
    cnt = 0;
    for (int i = 0; i < n3 - 1; i++) {
        cnt += line3[i + 1] - line3[i];
    }
    if (cnt != n2) {
        fprintf(stderr, "Blad krytyczny pliku wejsciowego\n");
        return ERR_FILE_CONNECTION_MISMATCH;
    }

    return VALIDATION_SUCCESS;
}
