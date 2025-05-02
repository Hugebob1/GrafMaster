#include "pliki.h" //czyli wszystko zwiazane z zapisywaniem/wypisywaniem/wczytywaniem
// Kody błędów (jeśli jeszcze nie ma)
#define ERR_SAVE_FILE_OPEN 5

int saveSubGraphs(GraphChunk* subgraphs, int numParts, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Nie mozna otworzyc pliku do zapisu");
        return ERR_SAVE_FILE_OPEN;
    }

    for (int i = 0; i < numParts; i++) {
        fprintf(file, "#%d\n", i + 1);

        for (int j = 0; j < subgraphs[i]->totalVertices; j++) {
            Vertex v = subgraphs[i]->vertices[j];
            if (!v) continue;  // tylko pomijaj NULL-e, nie puste

            fprintf(file, "%d:", v->id);
            for (int k = 0; k < v->degree; k++) {
                fprintf(file, " %d", v->edges[k]);
            }
            fprintf(file, " (%d,%d)\n", v->x, v->y);
        }
    }

    fclose(file);
    printf("Grafy zapisane do pliku: %s\n", filename);
    return 0;
}
#define ERR_SAVE_BIN_FILE_OPEN 6

int saveSubGraphsCompactBinary(GraphChunk* subgraphs, uint8_t numParts, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Nie mozna otworzyc pliku do zapisu");
        return ERR_SAVE_BIN_FILE_OPEN;
    }
    // NAGŁÓWEK 
    char signature[4] = {'S', 'U', 'B', 'G'}; // sygnatura pliku
    uint8_t version = 1;                     // wersja formatu

    fwrite(signature, sizeof(char), 4, file);     // zapisujemy sygnature
    fwrite(&version, sizeof(uint8_t), 1, file);   // zapisujemy wersje
    fwrite(&numParts, sizeof(uint8_t), 1, file);  // zapisujemy liczbe podgrafow
    for (uint8_t i = 0; i < numParts; i++) {
        GraphChunk g = subgraphs[i];

        uint16_t count = 0;
        for (int j = 0; j < g->totalVertices; j++) {
            if (g->vertices[j]) count++;
        }

        fwrite(&count, sizeof(uint16_t), 1, file);

        for (int j = 0; j < g->totalVertices; j++) {
            Vertex v = g->vertices[j];
            if (!v) continue;

            uint16_t id = (uint16_t)v->id;
            int16_t x = (int16_t)v->x;
            int16_t y = (int16_t)v->y;
            uint8_t deg = (uint8_t)v->degree;

            fwrite(&id, sizeof(uint16_t), 1, file);
            fwrite(&x, sizeof(int16_t), 1, file);
            fwrite(&y, sizeof(int16_t), 1, file);
            fwrite(&deg, sizeof(uint8_t), 1, file);

            for (int k = 0; k < deg; k++) {
                uint16_t neighbor = (uint16_t)v->edges[k];
                fwrite(&neighbor, sizeof(uint16_t), 1, file);
            }
        }
    }

    fclose(file);
    printf("Grafy zapisane do pliku binarnego: %s\n", filename);
    return 0;
}
GraphChunk* loadSubGraphsFromBinary(const char* filename, int* outNumParts) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie mozna otworzyc pliku binarnego");
        return NULL;
    }

    uint8_t numParts;
    fread(&numParts, sizeof(uint8_t), 1, file);
    *outNumParts = numParts;

    GraphChunk* parts = malloc(sizeof(GraphChunk) * numParts);

    for (int i = 0; i < numParts; i++) {
        uint16_t count;
        fread(&count, sizeof(uint16_t), 1, file);

        int maxId = -1;
        long start = ftell(file);

        for (int j = 0; j < count; j++) {
            uint16_t id;
            fread(&id, sizeof(uint16_t), 1, file);
            fseek(file, sizeof(int16_t) * 2, SEEK_CUR); // skip x, y
            uint8_t deg;
            fread(&deg, sizeof(uint8_t), 1, file);
            fseek(file, deg * sizeof(uint16_t), SEEK_CUR);

            if (id > maxId) maxId = id;
        }

        fseek(file, start, SEEK_SET);

        GraphChunk g = malloc(sizeof(struct GraphChunk));
        g->totalVertices = maxId + 1;
        g->vertices = calloc(g->totalVertices, sizeof(Vertex));

        for (int j = 0; j < count; j++) {
            uint16_t id;
            int16_t x, y;
            uint8_t deg;

            fread(&id, sizeof(uint16_t), 1, file);
            fread(&x, sizeof(int16_t), 1, file);
            fread(&y, sizeof(int16_t), 1, file);
            fread(&deg, sizeof(uint8_t), 1, file);

            Vertex v = createVertex(id, deg);
            v->x = x;
            v->y = y;
            v->degree = deg;

            for (int d = 0; d < deg; d++) {
                uint16_t neighbor;
                fread(&neighbor, sizeof(uint16_t), 1, file);
                v->edges[d] = neighbor;
            }

            g->vertices[id] = v;
        }

        parts[i] = g;
    }

    fclose(file);
    return parts;
}
GraphChunk loadGraphFromBinaryToChunk(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie mozna otworzyc pliku binarnego do odczytu");
        return NULL;
    }

    int numVertices = 0;
    fread(&numVertices, sizeof(int), 1, file);

    GraphChunk graph = malloc(sizeof(struct GraphChunk));
    graph->totalVertices = numVertices;
    graph->vertices = malloc(sizeof(Vertex) * numVertices);

    for (int i = 0; i < numVertices; i++) {
        uint8_t degree;
        fread(&degree, sizeof(uint8_t), 1, file);

        Vertex vertex = createVertex(i, degree);
        vertex->degree = degree;

        for (int j = 0; j < degree; j++) {
            uint16_t neighbor;
            fread(&neighbor, sizeof(uint16_t), 1, file);
            vertex->edges[j] = neighbor;
        }

        graph->vertices[i] = vertex;
    }

    fclose(file);
    return graph;
}
void saveGraphBinaryCompact(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Nie mozna otworzyc pliku binarnego");
        return;
    }
    int totalVertices = graph->totalVertices;
    fwrite(&totalVertices, sizeof(int), 1, file); // <<--- nagłówek
    for (int i=0;i<totalVertices;i++) {
        Vertex v = graph->vertices[i];
        uint8_t degree = (uint8_t) v->degree;
        fwrite(&degree, sizeof(uint8_t), 1, file);

        for (int j = 0; j < degree; j++) {
            uint16_t neighbor = (uint16_t) v->edges[j];
            fwrite(&neighbor, sizeof(uint16_t), 1, file);
        }
    }
    printf("Graf zapisany do pliku binarnego: %s\n", filename);
    fclose(file);
}
void exportGraph(GraphChunk graph, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Nie można otworzyć pliku do zapisu");
        return;
    }

    for (int i = 0; i < graph->totalVertices; i++) {
        Vertex v = graph->vertices[i];
        if (!v || v->degree == 0) continue;

        fprintf(file, "%d: ", v->id);
        for (int j = 0; j < v->degree; j++) {
            if(j==v->degree-1)
                fprintf(file, "%d\n", v->edges[j]);
            else
                fprintf(file, "%d ", v->edges[j]);
        }
    }

    fclose(file);
    printf("Graf zapisany do pliku: %s\n", filename);
}
