#ifndef PLIKI_H
#define PLIKI_H
#include "graph1.h"
#include  "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

int saveSubGraphs(GraphChunk* subgraphs, int numParts, const char* filename);
int saveSubGraphsCompactBinary(GraphChunk* subgraphs, uint16_t numParts, const char* filename);
GraphChunk* loadSubGraphsFromBinary(const char* filename, int* outNumParts);
GraphChunk loadGraphFromBinaryToChunk(const char* filename);
void saveGraphBinaryCompact(GraphChunk graph, const char* filename);
void exportGraph(GraphChunk graph, const char* filename);

#endif //PLIKI_H
