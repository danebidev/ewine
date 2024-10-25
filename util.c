#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void remove_last_path_component(const char* path) {
    // Loop to handle the case where there's multiple
    // slashes at the end of a path
    while (1) {
        char* last_slash = strrchr(path, '/');
        *last_slash = '\0';
        if (*(last_slash + 1) != '\0') break;
    }
}

char* read_file(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("error: failed to read file");
        return NULL;
    }

    // Get the size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(filesize + 1);  // +1 for null terminator
    if (!buffer) {
        printf("error: couldn't allocate memory");
        exit(1);
    }

    fread(buffer, 1, filesize, file);

    buffer[filesize] = '\0';

    fclose(file);
    return buffer;
}
