#include "graph.h"
#include "utils.h"
#include <time.h>

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
    
    
    GraphChunk graph = addEdges(argv[1]);
    //printGraphChunk(graph);
    if(validateGraphChunk(graph)){
        printf("Graf wczytano poprawnie\n");
    } else {
        printf("Bledy w strukturze grafu\n");
    }
    exportGraph(graph, "graph_original.csv");


    int numParts = 2;
    float maxDiff = 99.0f; // dopuszczalna różnica %

    GraphChunk* subgraphs = splitGraphBalancedConnected(graph, numParts, maxDiff);

    if (subgraphs) {
        for (int i = 0; i < numParts; i++) {
            char filename[64];
            sprintf(filename, "subgraph_%d.txt", i);
            exportGraph(subgraphs[i], filename);
        }
    }

    //saveGraphBinaryCompact(graph, "graph_original.bin");


    //test pamieci
    //printf("Tekstowy: %ld bajtow\n", getFileSize("graph_original.csv"));
    //printf("Binarny : %ld bajtow\n", getFileSize("graph_original.bin"));

   // GraphChunk graph2 = loadGraphFromBinaryToChunk("graph_original.bin");
    //printGraphChunk(graph2);

    freeGraphChunk(graph);
    //freeGraphChunk(graph2);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);
    return 0;

    //test binarnego
    //saveGraphBinary(graph, "graph_original.bin");
    //Graph* graph2 = loadGraphBinary("graph_original.bin");
    //exportGraph(graph2, "graph_original2.csv");

    //return 0;
}
