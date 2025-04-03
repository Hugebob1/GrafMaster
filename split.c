#include "split.h"
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
GraphChunk* splitGraphRetryIfNeeded(GraphChunk graph, int numParts, float maxDiffPercent) {
    GraphChunk* parts = splitGraphGreedyBalanced(graph, numParts, maxDiffPercent);
    if (!parts) return NULL;

    int total = graph->totalVertices;
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
            //printf("Podgraf %d jest pusty, probuje podzilic najwiekszy podgraf...\n", i);
            int maxIdx = 0, maxCount = 0;
            for (int j = 0; j < numParts; j++) {
                int count = 0;
                for (int v = 0; v < parts[j]->totalVertices; v++) {
                    if (parts[j]->vertices[v]) count++;
                }
                if (count > maxCount) {
                    maxCount = count;
                    maxIdx = j;
                }
            }
            //printf("Podgraf %d bedzie podzielony na 2 czesci...\n", maxIdx);
            GraphChunk* newParts = splitGraphGreedyBalanced(parts[maxIdx], 2, 1000.0f);
            parts[0] = newParts[0];
            parts[i] = newParts[1];
        }
    }
    for(int i=0;i<numParts;i++){
        if(isGraphConnected(parts[i])==false){

            printf("Podzial niemozliwy\n");
            return NULL;
        }
    }
    return parts;
}