#include "validation.h"
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

        // Bezpieczne sprawdzenie indeksu sąsiada
        if (neighborId < 0 || neighborId >= totalVertices) continue;
        if (!vertices[neighborId]) continue;

        if (!visited[neighborId]) {
            dfs(vertices, neighborId, visited, totalVertices);
        }
    }
}

bool isGraphConnected(GraphChunk graph) {
    if (!graph || !graph->vertices) {
        //printf("Blad: graf jest NULL\n");
        return false;
    }

    int totalVertices = graph->totalVertices;
    bool* visited = calloc(totalVertices, sizeof(bool));
    if (!visited) {
        perror("calloc failed");
        return false;
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
        //printf("Brak aktywnych wierzcholkow — graf pusty lub bez krawędzi\n");
        free(visited);
        return false;
    }

    dfs(graph->vertices, start, visited, totalVertices);

    // Sprawdź, czy wszystkie aktywne wierzchołki zostały odwiedzone
    for (int i = 0; i < totalVertices; i++) {
        if (graph->vertices[i] && graph->vertices[i]->degree > 0 && !visited[i]) {
            //printf("Wierzcholek %d nie zostal osiagniety graf niespojny\n", i);
            free(visited);
            return false;
        }
    }

    free(visited);
    return true;
}
bool validatefile(int vertecies, int* line2, int n2, int* line3, int n3, int* connectios, int numc, int* sections, int nums) {

    // 1. Sprawdzenie minimalnej liczby linii i niepustych linii
    if (vertecies <= 0 || n2 <= 0 || n3 <= 0 || numc <= 0 || nums <= 0) {
        fprintf(stderr, "Blad: plik zawiera puste linie lub za malo danych\n");
        return false;
    }
    int nonEmptyLines = 0;
    if (vertecies > 0) nonEmptyLines++;
    if (n2 > 0) nonEmptyLines++;
    if (n3 > 0) nonEmptyLines++;
    if (numc > 0) nonEmptyLines++;
    if (nums > 0) nonEmptyLines++;

    if (nonEmptyLines < 5) {
        fprintf(stderr, "Blad: plik musi zawierac minimum 5 niepustych linii\n");
        return false;
    }

    int cnt = 0;

    // 2. Sprawdzenie poprawności line3 (czy przedziały rosnące)
    for (int i = 0; i < n3 - 1; i++) {
        if (line3[i + 1] - line3[i] >= 0) {
            cnt++;
        } else if (line3[i + 1] - line3[i] < 0) {
            fprintf(stderr, "Blad przedzialu w pliku wejsciowym\n");
            return false;
        }
    }
    if (cnt != vertecies) {
        fprintf(stderr, "Braki w przedzialach w pliku wejsciowym\n");
        return false;
    }

    // 3. Sprawdzenie, czy suma przedzialow pasuje do liczby polaczen
    cnt = 0;
    for (int i = 0; i < n3 - 1; i++) {
        cnt += line3[i + 1] - line3[i];
    }
    if (cnt != n2) {
        fprintf(stderr, "Blad krytyczny pliku wejsciowego\n");
        return false;
    }

    return true;
}
