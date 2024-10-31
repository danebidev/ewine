#include "list.h"

#include <stdio.h>
#include <string.h>

#include "build_config.h"
#include "data.h"

void list_prefixes() {
    printf("Prefixes\n");
    for (int cur_prefix = 0; cur_prefix < data.prefix_count; cur_prefix++) {
        printf("    Name: %s\n", data.prefixes[cur_prefix].name);
        printf("    Path: %s\n", data.prefixes[cur_prefix].path);
        printf("    Wine: %s\n", data.prefixes[cur_prefix].wine);
        printf("    DXVK: %s\n", data.prefixes[cur_prefix].dxvk);
        printf("    Arch: %s\n", archstr(data.prefixes[cur_prefix].arch));
        if (cur_prefix < data.prefix_count - 1) printf("\n");
    }
}

void list_wine_installs() {
    printf("Wine\n");
    for (int cur_wine = 0; cur_wine < data.wine_count; cur_wine++) {
        printf("    Name: %s\n", data.wine_installs[cur_wine].name);
        printf("    Path: %s\n", data.wine_installs[cur_wine].path);
        if (cur_wine < data.wine_count - 1) printf("\n");
    }
}

void list_dxvk_installs() {
    printf("DXVK\n");
    for (int cur_dxvk = 0; cur_dxvk < data.dxvk_count; cur_dxvk++) {
        printf("    Name: %s\n", data.dxvk_installs[cur_dxvk].name);
        printf("    Path: %s\n", data.dxvk_installs[cur_dxvk].path);
        if (cur_dxvk < data.dxvk_count - 1) printf("\n");
    }
}

/**
 * @return 0 if the command completed succesfully, -1 otherwise
 */
int command_list(char *argv[], int argc, int cur_index) {
    if (cur_index++ >= argc) {
        list_prefixes();
        printf("\n");
        list_wine_installs();
        printf("\n");
        list_dxvk_installs();
    }
    else {
        for (; cur_index < argc; cur_index++) {
            if (strcmp(argv[cur_index], "prefix"))
                list_prefixes();
            else if (strcmp(argv[cur_index], "wine"))
                list_wine_installs();
            else if (strcmp(argv[cur_index], "dxvk"))
                list_dxvk_installs();
            else {
                printf("%s: Unrecognized component type '%s'. Should be one of 'prefix', 'wine' or 'dxvk'", PROGRAM_NAME, argv[cur_index]);
                return -1;
            }
        }
    }

    return 0;
}
