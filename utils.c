#include "utils.h"

// Function to count the number of numeric values in a semicolon-separated string
int how_many_digits(const char *line) {
    int count = 0;
    const char *p = line;
    while (*p) {
        if (*p == ';') {
            count++;
        }
        p++;
    }
    return (strlen(line) > 0) ? count + 1 : 0;
}

// Function to extract numbers from a semicolon-separated string
void read_digits(char *line4, int *connections) {
    char *token = strtok(line4, ";");
    int index = 0;
    while (token != NULL) {
        int is_number = 1;
        for (int i = 0; token[i] != '\0'; i++) {
            if (!isdigit(token[i])) {
                is_number = 0;
                break;
            }
        }
        if (is_number) {
            connections[index++] = atoi(token);
        }
        token = strtok(NULL, ";");
    }
}
