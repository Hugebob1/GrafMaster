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

int main(int argc, char **argv){
    clock_t start = clock();

    //domyslna wartosc x = 0
    int x = 0;
    int opt;
    int numParts = 2;
    float maxDiff = 10.0f;
    while ((opt = getopt(argc, argv, "hbag:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Instrukcja programu\n");
            return 0;
         /*   case 'b':
                strcpy(binaryname, optarg);  // Przypisanie nazwy pliku binarnego
            break;
            case 'a':
                strcpy(tekstowy, optarg);  // Przypisanie nazwy pliku tekstowego
            break;
            */
            case 'g':
                x = atof(argv[optind-1]);
                break;
            default:
                printf("Wpisana flaga nie istnieje\n");
            return 1;
        }
    }
   // printf("%s",argv[optind-1]);
    GraphChunk graph = addEdges(argv[optind], x);

    exportGraph(graph, "graph_original.csv");

    validateGraphChunk(graph);

    isGraphConnected(graph);

    saveGraphBinaryCompact(graph, "graph_original.bin");
    int n = 2;
    GraphChunk* parts = splitGraphRetryIfNeeded(graph, n, 1000.0f);

    saveSubGraphs(parts, n, "subgraphs.txt");
    saveSubGraphsCompactBinary(parts, n, "subgraphs.bin");

    freeGraphChunk(graph); 

    //testowalem czy wczytuje dobrze te pliki binarne
    // int xd;
    // GraphChunk* parts1 = loadSubGraphsFromBinary("subgraphs.bin", &xd);
    // printGraphChunk(parts1[0]); // lub inny podgraf


    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);

    printf("Rozmiar pliku csv: %ld KB\n", getFileSize("subgraphs.txt")/1024);
    printf("Rozmiar pliku bin: %ld KB\n", getFileSize("subgraphs.bin")/1024);

    return 0;
}