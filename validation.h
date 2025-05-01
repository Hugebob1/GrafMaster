#ifndef VALIDATION_H
#define VALIDATION_H
#include "graph1.h"
bool validateGraphChunk(GraphChunk graph);
void dfs(Vertex* vertices, int currentId, bool* visited, int totalVertices) ;
bool isGraphConnected(GraphChunk graph);
bool validatefile(int vertecies, int *line2, int n2, int *line3, int n3, int *connectios, int numc, int *sections, int nums);
#endif //VALIDATION_H
