#ifndef SPLIT_H
#define SPLIT_H
#include "graph1.h"
#include "validation.h"
int compareDegreeDesc(const void* a, const void* b);
GraphChunk* splitGraphGreedyBalanced(GraphChunk graph, int numParts, float maxDiffPercent);
GraphChunk* splitGraphRetryIfNeeded(GraphChunk graph, int numParts, float maxDiffPercent);
void getFinalDiff(GraphChunk *parts, int numParts, int total);
int balanceSubGraphs(GraphChunk original, GraphChunk *parts, int numParts, float maxDiffPercent, bool force);
#endif //SPLIT_H
