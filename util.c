#include "util.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"

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
        return NULL;
    }

    // Get the size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(filesize + 1);  // +1 for null terminator
    if (!buffer) {
        LOG(LOG_ERROR, "couldn't allocate memory");
        exit(1);
    }

    fread(buffer, 1, filesize, file);

    buffer[filesize] = '\0';

    fclose(file);
    return buffer;
}

int mkdirp(const char* path) {
    char tmp[PATH_MAX];
    char* p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}
