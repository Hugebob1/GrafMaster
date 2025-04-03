#ifndef SPLIT_H
#define SPLIT_H
#include "graph1.h"
#include "validation.h"
int compareDegreeDesc(const void* a, const void* b);
GraphChunk* splitGraphGreedyBalanced(GraphChunk graph, int numParts, float maxDiffPercent);
GraphChunk* splitGraphRetryIfNeeded(GraphChunk graph, int numParts, float maxDiffPercent);
#endif //SPLIT_H
