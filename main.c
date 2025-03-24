#include "graph.h"
#include "utils.h"

int main(int argc, char **argv) {
    

    
    GraphChunk graph = addEdges(argv[1]);
    printGraphChunk(graph);
    //makeGraphUndirected(graph);
    if(validateGraphChunk(graph)){
        printf("Graf wczytano poprawnie\n");
    } else {
        printf("Bledy w strukturze grafu\n");
    }
    //saveGraphUniqueUndirected(graph, "graph_original.csv");
    freeGraphChunk(graph);
    return 0;

    //exportGraph(graph, "graph_original.csv");  // Zapisz oryginalny graf
    // if (validateGraph(graph)) {
    //     printf("Graf wczytano poprawnie\n");
    // } else {
    //     printf("Bledy w strukturze grafu\n");
    // }
    //test binarnego
    //saveGraphBinary(graph, "graph_original.bin");
    //Graph* graph2 = loadGraphBinary("graph_original.bin");
    //exportGraph(graph2, "graph_original2.csv");

    //return 0;
}
