#include "utils.h"

// Function to count the number of numeric values in a semicolon-separated string
int numElements(const char *filename, int targetLine) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Nie można otworzyć pliku");
        exit(1);
    }

    int line = 1;
    int count = 0;
    int value;

    while (fscanf(fp, "%d", &value) == 1) {
        if (line == targetLine) {
            count++;
        }
        if (fgetc(fp) == '\n') {
            line++;
        }
    }

    fclose(fp);
    return count;
}

int numLines(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Nie można otworzyć pliku");
        exit(1);
    }
    int line = 1;
    int value;

    while (fscanf(fp, "%d", &value) == 1) {
        if (fgetc(fp) == '\n') {
            line++;
        }
    }

    fclose(fp);
    return line;
}


/*function reads the line and returns an array*/
int* readLine(const char *filename, int targetLine, int expectedCount) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Błąd podczas otwierania pliku");
        exit(2137);
    }

    int currentLine = 1;
    int count = 0;
    int value;

    int *result = malloc(sizeof(int) * expectedCount);
    if (!result) {
        perror("Błąd alokacji pamięci");
        exit(2137);
    }

    while (fscanf(fp, "%d", &value) == 1) {
        if (currentLine == targetLine) {
            result[count++] = value;
            if (count >= expectedCount) break;
        }
        if (fgetc(fp) == '\n') {
            currentLine++;
        }
    }

    fclose(fp);
    return result;
}

