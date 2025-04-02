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
    int opt;
    int numParts = 2;
    char binaryname[100]="subgraphs.bin";
    char tekstowy[100]="subgraphs.txt";
    float maxDiff = 10.0f;
    while ((opt = getopt(argc, argv, "hb:a:g:p:d:f")) != -1) { //a.exe graf.txt -g 1 -p 2 -d 60 (w skrócie flaga argument, flaga argument_
        switch (opt) {
            case 'h':
                printf("Instrukcja programu\n");
            return 0;
          case 'b':
                strcpy(binaryname, optarg);  // Przypisanie nazwy pliku binarnego
            break;
            case 'a':
                strcpy(tekstowy, optarg);  // Przypisanie nazwy pliku tekstowego
            break;
            case 'g':
                if (optarg) {
                    x = atof(optarg);
                }
            break;
            case 'p':
                if (optarg) {
                    numParts = atof(optarg);
                }
            break;
            case 'd':
                if (optarg) {
                    maxDiff = atof(optarg);
                }
            break;
            case 'f':
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
    GraphChunk* parts = splitGraphGreedyBalanced(graph, numParts, maxDiff);

    saveSubGraphs(parts, numParts, tekstowy);
    saveSubGraphsCompactBinary(parts,numParts, binaryname);
    isGraphConnected(parts[0]);
    isGraphConnected(parts[1]);

    freeGraphChunk(graph); 

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);

    printf("Rozmiar pliku csv: %ld KB\n", getFileSize(tekstowy)/1024);
    printf("Rozmiar pliku bin: %ld KB\n", getFileSize(binaryname)/1024);

    return 0;
}