#include "graph1.h"
#include "utils.h"
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>

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

    GraphChunk graph = addEdges(argv[1], x);

    exportGraph(graph, "graph_original.csv");

    validateGraphChunk(graph);

    isGraphConnected(graph);

    saveGraphBinaryCompact(graph, "graph_original.bin");

    GraphChunk* parts = splitGraphGreedyBalanced(graph, 3, 86.0f);

    saveSubGraphs(parts, 3, "subgraphs.txt");
    saveSubGraphsCompactBinary(parts, 3, "subgraphs.bin");

    isGraphConnected(parts[0]);
    isGraphConnected(parts[1]);

    freeGraphChunk(graph); 

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);

    printf("Rozmiar pliku csv: %ld KB\n", getFileSize("subgraphs.txt")/1024);
    printf("Rozmiar pliku bin: %ld KB\n", getFileSize("subgraphs.bin")/1024);

    return 0;
}