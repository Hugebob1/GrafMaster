#ifndef VALIDATION_H
#define VALIDATION_H
#include "graph1.h"
int validateGraphChunk(GraphChunk graph);
void dfs(Vertex* vertices, int currentId, bool* visited, int totalVertices) ;
int isGraphConnected(GraphChunk graph);
int validatefile(int vertecies, int *line2, int n2, int *line3, int n3, int *connectios, int numc, int *sections, int nums);
#endif //VALIDATION_H
