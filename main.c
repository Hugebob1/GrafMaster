#include "graph.h"
#include "utils.h"
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
// a.exe -a pomocniczy -b binarnazagrywaka.bin graf.txt 2 50 przyklad flagi
// a.exe graf.txt 2 50        bez flag
long getFileSize(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Nie mozna otworzyc pliku");
        return -1;
    }

    fseek(file, 0, SEEK_END);       // Przeskocz na koniec pliku
    long size = ftell(file);        // Pobierz pozycję wskaźnika = rozmiar
    fclose(file);
    return size;
}

int main(int argc, char **argv) {
    clock_t start = clock();

    // Inicjalizacja zmiennych
    char binaryname[64] = "graph_original.bin";
    char tekstowy[64] = "graph";
    int numParts = 2;  // Domyślna wartość numParts
    float maxDiff = 99.0f;  // Domyślna wartość maxDiff
    int opt;

    // Przetwarzanie flag wiersza poleceń
    while ((opt = getopt(argc, argv, "hb:a:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Instrukcja programu\n");
                return 0;
            case 'b':
                strcpy(binaryname, optarg);  // Przypisanie nazwy pliku binarnego
                break;
            case 'a':
                strcpy(tekstowy, optarg);  // Przypisanie nazwy pliku tekstowego
                break;
            default:
                printf("Wpisana flaga nie istnieje\n");
                return 1;
        }
    }

    // Sprawdzanie, czy podano plik wejściowy
    if (optind >= argc) {
        printf("Nie podano pliku wejściowego\n");
        return 1;
    }

    // Odczyt numParts i maxDiff z argumentów po pliku wejściowym
    numParts = atoi(argv[optind+1]);  // Pierwszy argument po pliku wejściowym to liczba podgrafów
    maxDiff = atof(argv[optind + 2]);   // Drugi argument to dopuszczalna różnica procentowa

    // Dodawanie krawędzi do grafu
    GraphChunk graph = addEdges(argv[optind]); // Plik wejściowy
    printf("%s",argv[optind - 1]);

    // Sprawdzanie poprawności grafu
    if (validateGraphChunk(graph)) {
        printf("Graf wczytano poprawnie\n");
    } else {
        printf("Bledy w strukturze grafu\n");
    }

    // Eksport grafu do pliku tekstowego
    exportGraph(graph, "graph_original.csv");

    // Tworzenie podgrafów
    GraphChunk* subgraphs = splitGraphBalancedConnected(graph, numParts, maxDiff);

    // Eksportowanie podgrafów do plików
    if (subgraphs) {
        for (int i = 0; i < numParts; i++) {
            char filename[64];
            // Tworzymy nazwę pliku na podstawie zmiennej tekstowy i numeru podgrafu
            sprintf(filename, "%s_%d.txt", tekstowy, i);
            exportGraph(subgraphs[i], filename);
        }
    }

    // Zapisanie grafu w formacie binarnym
    saveGraphBinaryCompact(graph, binaryname);

    // Zwalnianie pamięci
    freeGraphChunk(graph);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Czas dzialania: %.6f sekund\n", time_spent);

    return 0;
}
