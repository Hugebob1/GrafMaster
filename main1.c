#include "graph1.h"
#include "utils.h"
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include "pliki.h"
#include "split.h"
#include "validation.h"
long getFileSize(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie mozna otworzyc pliku");
        return -1;
    }

    fseek(file, 0, SEEK_END);       // Przeskocz na koniec pliku
    long size = ftell(file);        // Pobierz pozycję wskaźnika = rozmiar
    fclose(file);
    return size;
}

int main(int argc, char **argv) {
    clock_t start = clock();

    int x = 0;
    int opt;
    int numParts = 2;
    float maxDiff = 10.0f;
    bool forceSplit = false;

    char tekstowy[100] = "subgraphs.txt";
    char binaryname[100] = "subgraphs.bin";

    while ((opt = getopt(argc, argv, "a:b:d:p:fg:h")) != -1) {
        switch (opt) {
            case 'a':
                strcpy(tekstowy, optarg);  // nazwa pliku tekstowego
                break;
            case 'b':
                strcpy(binaryname, optarg);  // nazwa pliku binarnego
                break;
            case 'd':
                maxDiff = atof(optarg);  // różnica procentowa
                break;
            case 'p':
                numParts = atoi(optarg);  // liczba podgrafów
                break;
            case 'f':
                forceSplit = true;  // wymuszenie podziału
                break;
            case 'g':
                x = atoi(optarg);  // numer grafu z pliku
                break;
            case 'h':
                printf("Uzycie: ./a.out <plik> [-g numer] [-p liczba] [-d procent] [-a plik.txt] [-b plik.bin] [-f]\n");
                return 0;
            default:
                fprintf(stderr, "Nieznana flaga. Uzyj -h po pomoc.\n");
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Brak pliku z grafem.\n");
        return 1;
    }

    GraphChunk graph = addEdges(argv[optind], x);
    //exportGraph(graph, "graph_original.csv");
    validateGraphChunk(graph);
    isGraphConnected(graph);
    //saveGraphBinaryCompact(graph, "graph_original.bin");

    GraphChunk* parts = splitGraphRetryIfNeeded(graph, numParts, maxDiff);

    if (parts == NULL && forceSplit) {
        for (int i = 2; i < graph->totalVertices; i++) {
            parts = splitGraphRetryIfNeeded(graph, i, 100000.0f);
            if (parts != NULL) {
                numParts = i;
                printf("Wymuszony podzial: %d podgrafow\n", numParts);
                break;
            }
        }
    }

    if (!parts) {
        fprintf(stderr, "Nie udalo sie podzielic grafu.\n");
        freeGraphChunk(graph);
        return 1;
    }

    saveSubGraphs(parts, numParts, tekstowy);
    saveSubGraphsCompactBinary(parts, numParts, binaryname);

    for (int i = 0; i < numParts; i++) {
        freeGraphChunk(parts[i]);
    }
    free(parts);
    freeGraphChunk(graph);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);
    printf("Rozmiar pliku csv: %ld KB\n", getFileSize(tekstowy)/1024);
    printf("Rozmiar pliku bin: %ld KB\n", getFileSize(binaryname)/1024);

    return 0;
}
