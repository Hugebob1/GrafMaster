#include "graph.h"
#include "utils.h"

int main(int argc, char **argv) {
    FILE *in = argc > 1 ? fopen(argv[1], "r") : stdin;
    if (!in) {
        perror("File opening failed");
        return 1;
    }

    Graph* graph = loadGraph(in);

    printGraph(graph);

    return 0;
}
