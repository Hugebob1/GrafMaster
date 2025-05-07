#include "split.h"
static int* degreeCmpHelper = NULL;

#define GRAPH_CONNECTED 0

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
    // if (diff > maxDiffPercent) {
    //     printf("Blad: roznica wielkosci grup przekracza %.2f%% (aktualnie: %.2f%%)\n", maxDiffPercent, diff);
    //     goto fail;
    // }

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

        // kopiowanie wspolrzednych x/y
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
        if(isGraphConnected(parts[i])!=GRAPH_CONNECTED){

            printf("Podzial niemozliwy\n");
            return NULL;
        }
    }
    return parts;
}
void getFinalDiff(GraphChunk *parts, int numParts, int total) {
    if(parts!=NULL){
        int minSize = total, maxSize = 0;
        int* partSizes = calloc(numParts, sizeof(int));
        if (!partSizes) {
            perror("calloc failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < numParts; i++) {
            for (int j = 0; j < total; j++) {
                if (parts[i]->vertices[j]) partSizes[i]++;
            }
        }

        for (int i = 0; i < numParts; i++) {
            if (partSizes[i] < minSize) minSize = partSizes[i];
            if (partSizes[i] > maxSize) maxSize = partSizes[i];
        }

        float baseSize = total / (float)numParts;
        float diff = (maxSize - minSize) / baseSize * 100.0f;
        printf("Roznica wierzcholkow: %.2f%%\n", diff);

        free(partSizes);
    }
}


float getFinalDiffvalue(GraphChunk *parts, int numParts, int total) {
    int minSize = total, maxSize = 0;
    if(parts==NULL){
        return 10e5f;
    }
    int* partSizes = calloc(numParts, sizeof(int));
    if (!partSizes) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numParts; i++) {
        for (int j = 0; j < total; j++) {
            if (parts[i]->vertices[j]) partSizes[i]++;
        }
    }

    for (int i = 0; i < numParts; i++) {
        if (partSizes[i] < minSize) minSize = partSizes[i];
        if (partSizes[i] > maxSize) maxSize = partSizes[i];
    }

    free(partSizes);

    float baseSize = total / (float)numParts;
    float diff = (maxSize - minSize) / baseSize * 100.0f;
    return diff;
}

int balanceSubGraphs(GraphChunk original, GraphChunk *parts, int numParts, float maxDiffPercent, bool force) {
    int total = original->totalVertices;
    int* partSizes = calloc(numParts, sizeof(int));

    for (int i = 0; i < numParts; i++) {
        for (int j = 0; j < total; j++) {
            if (parts[i]->vertices[j]) partSizes[i]++;
        }
    }

    int baseSize = total / numParts;
    int maxIter = total * 2; // limit bezpieczeństwa
    int noProgress = 0;
    int maxNoProgress = 50;  // maksymalnie 50 pustych prób

    for (int iter = 0; iter < maxIter; iter++) {
        // Szukamy najbardziej niezbalansowanych par
        int maxIdx = 0, minIdx = 0;
        for (int i = 1; i < numParts; i++) {
            if (partSizes[i] > partSizes[maxIdx]) maxIdx = i;
            if (partSizes[i] < partSizes[minIdx]) minIdx = i;
        }

        float diff = (partSizes[maxIdx] - partSizes[minIdx]) / (float)baseSize * 100.0f;
        if (diff <= maxDiffPercent) break;

        // Szukamy kandydata do przeniesienia
        int bestVertex = -1;
        int maxBenefit = -1;

        for (int v = 0; v < total; v++) {
            if (!parts[maxIdx]->vertices[v]) continue;
            Vertex orig = original->vertices[v];
            if (!orig) continue;

            int connectedToMin = 0;
            for (int i = 0; i < orig->degree; i++) {
                int nei = orig->edges[i];
                if (nei >= 0 && nei < total && parts[minIdx]->vertices[nei]) {
                    connectedToMin++;
                }
            }

            if (connectedToMin > maxBenefit) {
                bestVertex = v;
                maxBenefit = connectedToMin;
            }
        }

        if (bestVertex == -1 || maxBenefit < 1) {
            noProgress++;
            if (noProgress > maxNoProgress) break;
            continue;
        }
        noProgress = 0; // udało się znaleźć kandydata

        Vertex orig = original->vertices[bestVertex];

        bool unsafeToMove = false;
        for (int i = 0; i < orig->degree; i++) {
            int nei = orig->edges[i];
            if (nei >= 0 && nei < total && parts[maxIdx]->vertices[nei]) {
                Vertex neighbor = parts[maxIdx]->vertices[nei];
                int otherConnections = 0;
                for (int j = 0; j < neighbor->degree; j++) {
                    int other = neighbor->edges[j];
                    if (other != bestVertex && parts[maxIdx]->vertices[other]) {
                        otherConnections++;
                    }
                }
                if (otherConnections == 0) {
                    unsafeToMove = true;
                    break;
                }
            }
        }

        // Ułatwiamy tylko dla dużych podziałów
        bool allowUnsafeMove = (numParts > 5);
        if (!allowUnsafeMove && unsafeToMove) continue;

        // Tworzymy nowy wierzchołek
        Vertex newV = createVertex(orig->id, orig->degree);
        newV->x = orig->x;
        newV->y = orig->y;
        newV->degree = 0;

        for (int i = 0; i < orig->degree; i++) {
            int nei = orig->edges[i];
            if (nei >= 0 && nei < total && parts[minIdx]->vertices[nei]) {
                newV->edges[newV->degree++] = nei;
                Vertex neighbor = parts[minIdx]->vertices[nei];
                bool alreadyConnected = false;
                for (int c = 0; c < neighbor->degree; c++) {
                    if (neighbor->edges[c] == bestVertex) {
                        alreadyConnected = true;
                        break;
                    }
                }
                if (!alreadyConnected) {
                    if (neighbor->degree >= neighbor->numEdges) {
                        neighbor->numEdges *= 2;
                        neighbor->edges = realloc(neighbor->edges, neighbor->numEdges * sizeof(int));
                    }
                    neighbor->edges[neighbor->degree++] = bestVertex;
                }
            }
        }

        if (newV->degree == 0) {
            free(newV->edges);
            free(newV);
            break;
        }

        Vertex old = parts[maxIdx]->vertices[bestVertex];
        parts[maxIdx]->vertices[bestVertex] = NULL;
        for (int i = 0; i < old->degree; i++) {
            int nei = old->edges[i];
            Vertex neighbor = parts[maxIdx]->vertices[nei];
            if (neighbor) {
                for (int j = 0; j < neighbor->degree; j++) {
                    if (neighbor->edges[j] == bestVertex) {
                        neighbor->edges[j] = neighbor->edges[--neighbor->degree];
                        break;
                    }
                }
            }
        }

        if (isGraphConnected(parts[maxIdx])!= GRAPH_CONNECTED) {
            // Przywracamy wszystko
            parts[maxIdx]->vertices[bestVertex] = old;
            for (int i = 0; i < newV->degree; i++) {
                int nei = newV->edges[i];
                Vertex neighbor = parts[minIdx]->vertices[nei];
                if (neighbor) {
                    for (int j = 0; j < neighbor->degree; j++) {
                        if (neighbor->edges[j] == bestVertex) {
                            neighbor->edges[j] = neighbor->edges[--neighbor->degree];
                            break;
                        }
                    }
                }
            }
            free(newV->edges);
            free(newV);
            continue;
        }

        free(old->edges);
        free(old);
        parts[minIdx]->vertices[bestVertex] = newV;
        partSizes[minIdx]++;
        partSizes[maxIdx]--;
    }

    int minEnd = total, maxEnd = 0;
    for (int i = 0; i < numParts; i++) {
        if (partSizes[i] < minEnd) minEnd = partSizes[i];
        if (partSizes[i] > maxEnd) maxEnd = partSizes[i];
    }
    float finalDiff = (maxEnd - minEnd) / (float)baseSize * 100.0f;

    free(partSizes);

    if (finalDiff > maxDiffPercent && !force) {
        for (int i = 0; i < numParts; i++) {
            freeGraphChunk(parts[i]);
        }
        return -1;
    } else {
        return 1;
    }
}
int balanceSubGraphsTurbo(GraphChunk original, GraphChunk *parts, int numParts, float maxDiffPercent, bool force) {
    int total = original->totalVertices;
    int* partSizes = calloc(numParts, sizeof(int));

    for (int i = 0; i < numParts; i++) {
        for (int j = 0; j < total; j++) {
            if (parts[i]->vertices[j]) partSizes[i]++;
        }
    }

    int baseSize = total / numParts;
    int maxIter = total * 3;  // więcej prób niż normalnie
    int noProgress = 0;
    int maxNoProgress = 100;  // więcej tolerancji

    for (int iter = 0; iter < maxIter; iter++) {
        int maxIdx = 0, minIdx = 0;
        for (int i = 1; i < numParts; i++) {
            if (partSizes[i] > partSizes[maxIdx]) maxIdx = i;
            if (partSizes[i] < partSizes[minIdx]) minIdx = i;
        }

        float diff = (partSizes[maxIdx] - partSizes[minIdx]) / (float)baseSize * 100.0f;
        if (diff <= maxDiffPercent) break;

        // Szukamy grupki kandydatów
        int* candidates = calloc(total, sizeof(int));
        int candidateCount = 0;

        for (int v = 0; v < total; v++) {
            if (!parts[maxIdx]->vertices[v]) continue;
            Vertex orig = original->vertices[v];
            if (!orig) continue;

            int connectedToMin = 0;
            for (int i = 0; i < orig->degree; i++) {
                int nei = orig->edges[i];
                if (nei >= 0 && nei < total && parts[minIdx]->vertices[nei]) {
                    connectedToMin++;
                }
            }
            if (connectedToMin >= 1) {
                candidates[candidateCount++] = v;
            }
            if (candidateCount >= 5) break; // maks 5 kandydatów naraz
        }

        if (candidateCount == 0) {
            free(candidates);
            noProgress++;
            if (noProgress > maxNoProgress) break;
            continue;
        }
        noProgress = 0;

        int moved = 0;
        for (int c = 0; c < candidateCount; c++) {
            int bestVertex = candidates[c];
            Vertex orig = original->vertices[bestVertex];

            // Sprawdzamy czy bezpiecznie usunąć
            bool unsafeToMove = false;
            for (int i = 0; i < orig->degree; i++) {
                int nei = orig->edges[i];
                if (nei >= 0 && nei < total && parts[maxIdx]->vertices[nei]) {
                    Vertex neighbor = parts[maxIdx]->vertices[nei];
                    int otherConnections = 0;
                    for (int j = 0; j < neighbor->degree; j++) {
                        int other = neighbor->edges[j];
                        if (other != bestVertex && parts[maxIdx]->vertices[other]) {
                            otherConnections++;
                        }
                    }
                    if (otherConnections == 0) {
                        unsafeToMove = true;
                        break;
                    }
                }
            }

            bool allowUnsafeMove = (numParts > 5);
            if (!allowUnsafeMove && unsafeToMove) continue;

            // Tworzymy nowy wierzchołek
            Vertex newV = createVertex(orig->id, orig->degree);
            newV->x = orig->x;
            newV->y = orig->y;
            newV->degree = 0;

            for (int i = 0; i < orig->degree; i++) {
                int nei = orig->edges[i];
                if (nei >= 0 && nei < total && parts[minIdx]->vertices[nei]) {
                    newV->edges[newV->degree++] = nei;
                    Vertex neighbor = parts[minIdx]->vertices[nei];
                    bool alreadyConnected = false;
                    for (int c = 0; c < neighbor->degree; c++) {
                        if (neighbor->edges[c] == bestVertex) {
                            alreadyConnected = true;
                            break;
                        }
                    }
                    if (!alreadyConnected) {
                        if (neighbor->degree >= neighbor->numEdges) {
                            neighbor->numEdges *= 2;
                            neighbor->edges = realloc(neighbor->edges, neighbor->numEdges * sizeof(int));
                        }
                        neighbor->edges[neighbor->degree++] = bestVertex;
                    }
                }
            }

            if (newV->degree == 0) {
                free(newV->edges);
                free(newV);
                continue;
            }

            Vertex old = parts[maxIdx]->vertices[bestVertex];
            parts[maxIdx]->vertices[bestVertex] = NULL;
            for (int i = 0; i < old->degree; i++) {
                int nei = old->edges[i];
                Vertex neighbor = parts[maxIdx]->vertices[nei];
                if (neighbor) {
                    for (int j = 0; j < neighbor->degree; j++) {
                        if (neighbor->edges[j] == bestVertex) {
                            neighbor->edges[j] = neighbor->edges[--neighbor->degree];
                            break;
                        }
                    }
                }
            }

            if (isGraphConnected(parts[maxIdx])!= GRAPH_CONNECTED) {
                // Przywróć
                parts[maxIdx]->vertices[bestVertex] = old;
                for (int i = 0; i < newV->degree; i++) {
                    int nei = newV->edges[i];
                    Vertex neighbor = parts[minIdx]->vertices[nei];
                    if (neighbor) {
                        for (int j = 0; j < neighbor->degree; j++) {
                            if (neighbor->edges[j] == bestVertex) {
                                neighbor->edges[j] = neighbor->edges[--neighbor->degree];
                                break;
                            }
                        }
                    }
                }
                free(newV->edges);
                free(newV);
                continue;
            }

            free(old->edges);
            free(old);
            parts[minIdx]->vertices[bestVertex] = newV;
            partSizes[minIdx]++;
            partSizes[maxIdx]--;
            moved++;
        }

        free(candidates);

        if (moved == 0) {
            noProgress++;
            if (noProgress > maxNoProgress) break;
        }
    }

    int minEnd = total, maxEnd = 0;
    for (int i = 0; i < numParts; i++) {
        if (partSizes[i] < minEnd) minEnd = partSizes[i];
        if (partSizes[i] > maxEnd) maxEnd = partSizes[i];
    }
    float finalDiff = (maxEnd - minEnd) / (float)baseSize * 100.0f;

    free(partSizes);

    if (finalDiff > maxDiffPercent && !force) {
        for (int i = 0; i < numParts; i++) {
            freeGraphChunk(parts[i]);
        }
        return -1;
    } else {
        return 1;
    }
}