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

prefix_t* get_prefix(char* prefix_name) {
    prefix_t* prefix = NULL;

    for (int i = 0; i < data.prefix_count; i++) {
        if (strcmp(data.prefixes[i].name, prefix_name) == 0) {
            prefix = &data.prefixes[i];
            break;
        }
    }

    return prefix;
}

int run(prefix_t* prefix) {
    if (apply_dxvk(prefix) == -1) {
        LOG(LOG_ERROR, "failed to apply dxvk setting\n");
        return -1;
    }

    if (!prefix->wine) {
        LOG(LOG_ERROR, "prefix '%s' doesn't have a valid wine version\n", prefix->name);
        return -1;
    }
    int wine_pid = fork();
    if (wine_pid == -1) {
        LOG(LOG_ERROR, "failed to fork to run wine\n");
        return -1;
    }

    if (wine_pid == 0) {
        int wine_length = strlen(prefix->wine->path) + 8;
        char* wine_path = malloc(wine_length);
        snprintf(wine_path, wine_length, "%s/wine64", prefix->wine->path);
        printf("%s", wine_path);

        setenv("WINEPREFIX", prefix->path, 0);

        char* args[] = { "wine64", prefix->binary, NULL };
        execv(wine_path, args);
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
    prefix_t* prefix = get_prefix(prefix_name);
    if (!prefix) LOG(LOG_ERROR, "no prefix named '%s'\n", prefix_name);

    if (run(prefix) == -1) {
        LOG(LOG_ERROR, "can't run prefix '%s'\n", prefix_name);
        return -1;
    }

    return 0;
}
