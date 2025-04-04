#ifndef VALIDATION_H
#define VALIDATION_H
#include "graph1.h"
bool validateGraphChunk(GraphChunk graph);
void dfs(Vertex* vertices, int currentId, bool* visited, int totalVertices) ;
bool isGraphConnected(GraphChunk graph);
#endif //VALIDATION_H
