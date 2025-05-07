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

    fseek(file, 0, SEEK_END);
    long size = ftell(file);        
    fclose(file);
    return size;
}

#define ERR_UNKNOWN_FLAG 1
#define ERR_NO_INPUT_FILE 2
#define ERR_INVALID_GRAPH 3
#define ERR_GRAPH_SPLIT_FAIL 4
#define ERR_SAVE_BIN_FILE_OPEN 6
#define GRAPH_CONNECTED 0

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
                if (!optarg) {
                    fprintf(stderr, "Brak argumentu dla -a (plik tekstowy)\n");
                    return ERR_UNKNOWN_FLAG;
                }
                strncpy(tekstowy, optarg, sizeof(tekstowy) - 1);
                tekstowy[sizeof(tekstowy) - 1] = '\0';
                break;
    
            case 'b':
                if (!optarg) {
                    fprintf(stderr, "Brak argumentu dla -b (plik binarny)\n");
                    return ERR_UNKNOWN_FLAG;
                }
                strncpy(binaryname, optarg, sizeof(binaryname) - 1);
                binaryname[sizeof(binaryname) - 1] = '\0';
                break;
    
            case 'd':
                if (!optarg) {
                    fprintf(stderr, "Brak argumentu dla -d (procent)\n");
                    return ERR_UNKNOWN_FLAG;
                }
                maxDiff = atof(optarg);
                if (maxDiff < 0.0f) {
                    fprintf(stderr, "Procent roznicy nie moze byc ujemny: %s\n", optarg);
                    return ERR_UNKNOWN_FLAG;
                }
                break;
    
            case 'p':
                if (!optarg) {
                    fprintf(stderr, "Brak argumentu dla -p (liczba podgrafow)\n");
                    return ERR_UNKNOWN_FLAG;
                }
                numParts = atoi(optarg);
                if (numParts <= 0) {
                    fprintf(stderr, "Liczba podgrafow musi byc dodatnia: %s\n", optarg);
                    return ERR_UNKNOWN_FLAG;
                }
                break;
    
            case 'f':
                forceSplit = true;
                break;
    
            case 'g':
                if (!optarg) {
                    fprintf(stderr, "Brak argumentu dla -g (parametr x)\n");
                    return ERR_UNKNOWN_FLAG;
                }
                x = atoi(optarg);
                break;
    
            case 'h':
                printf("Mozliwe opcje: ./a.out [-g numer] [-p liczba] [-d procent] [-a plik.txt] [-b plik.bin] [-f] <plik>\n");
                return 0;
    
            default:
                fprintf(stderr, "Nieznana flaga. Uzyj -h po pomoc.\n");
                return ERR_UNKNOWN_FLAG;
        }
    }    

    if (optind >= argc) {
        fprintf(stderr, "Brak pliku z grafem.\n");
        return ERR_NO_INPUT_FILE;
    }

    GraphChunk graph = addEdges(argv[optind], x);
    if (!graph) {
        fprintf(stderr, "Blad podczas tworzenia grafu. Kod bledu: %d\n", lastgrapherror);
        return lastgrapherror;
    }
    int addedgeserrors = edgesErrors;
    if (addedgeserrors != 0) {
        fprintf(stderr, "Blad podczas dodawania krawedzi. Kod bledu: %d\n", addedgeserrors);
        freeGraphChunk(graph);
        return addedgeserrors;
    }
    int pomstatus = validateGraphChunk(graph);
    if(pomstatus!=0){
        return pomstatus;
    }
    if (isGraphConnected(graph)!= GRAPH_CONNECTED) {
        fprintf(stderr ,"Bledny zapis grafu w pliku wejsciowm\n");
        return ERR_INVALID_GRAPH;
    }
    GraphChunk* parts = splitGraphRetryIfNeeded(graph, numParts, maxDiff);
    bool czyweszlo = false;
    if ((parts == NULL && forceSplit)) {
        for (int i = 2; i < graph->totalVertices; i++) {
            parts = splitGraphRetryIfNeeded(graph, i, 100000.0f);
            if (parts != NULL) {
                numParts = i;
                printf("Wymuszony podzial: %d podgrafow\n", numParts);
                czyweszlo = true;
                getFinalDiff(parts, numParts, graph->totalVertices);
                break;
            }
        }
    }

    if ((parts==NULL || getFinalDiffvalue(parts, numParts, graph->totalVertices) > maxDiff) && !forceSplit) {
        fprintf(stderr, "Nie udalo sie podzielic grafu.\n");
        freeGraphChunk(graph);
        return ERR_GRAPH_SPLIT_FAIL;
    }

    if (!czyweszlo) {
        getFinalDiff(parts, numParts, graph->totalVertices);
    }

    int code = saveSubGraphs(parts, numParts, tekstowy);
    if (code != 0) return code;

    if (saveSubGraphsCompactBinary(parts, numParts, binaryname) != 0) return ERR_SAVE_BIN_FILE_OPEN;

    for (int i = 0; i < numParts; i++) {
        freeGraphChunk(parts[i]);
    }
    free(parts);
    freeGraphChunk(graph);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);
    printf("Rozmiar pliku txt: %ld B\n", getFileSize(tekstowy));
    printf("Rozmiar pliku bin: %ld B\n", getFileSize(binaryname));

    return 0;
}

