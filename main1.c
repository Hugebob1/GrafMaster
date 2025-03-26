#include "graph1.h"
#include "utils.h"
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv){
    clock_t start = clock();
    GraphChunk graph = addEdges(argv[1]);
    exportGraph(graph, "graph_original.csv");
    //printGraphChunk(graph);   
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);
}