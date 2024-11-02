#include "run.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "build_config.h"
#include "config.h"
#include "data.h"
#include "util.h"

int apply_dxvk(prefix_t* _) {
    return 0;
}

int check_wine_binary(wine_t wine, char* name) {
    int strsize = strlen(wine.path) + strlen(name) + 2;
    char* path = malloc(strsize);
    if (!path) {
        LOG(LOG_ERROR, "memory allocation failed for wine binary path");
        return -1;
    }

    snprintf(path, strsize, "%s/%s", wine.path, name);

    free(path);
    return 0;
}

int run(prefix_t* prefix) {
    if (apply_dxvk(prefix) == -1) {
        LOG(LOG_ERROR, "failed to apply dxvk setting");
        return -1;
    }

    return 0;
}

int command_run(char** argv, int argc, int args_index) {
    if (args_index == argc) {
        printf("%s: Missing prefix name.\n", PROGRAM_NAME);
        return -1;
    }
    if (args_index + 1 != argc) {
        printf("%s: Unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index + 1]);
        return -1;
    }

    char* prefix_name = argv[args_index];
    prefix_t* prefix = NULL;

    for (int i = 0; i < data.prefix_count; i++) {
        if (strcmp(data.prefixes[i].name, prefix_name) == 0) {
            prefix = &data.prefixes[i];
            break;
        }
    }

    if (prefix == NULL) {
        LOG(LOG_ERROR, "no prefix named '%s'\n", prefix_name);
    }

    run(prefix);

    return 0;
}
