#include "list.h"

#include <stdio.h>
#include <string.h>

#include "build_config.h"
#include "data.h"

int list_prefixes() {
    printf("Prefixes\n");
    if (data.prefix_count) {
        for (int cur_prefix = 0; cur_prefix < data.prefix_count; cur_prefix++) {
            printf("    Name: %s\n", data.prefixes[cur_prefix].name);
            printf("    Path: %s\n", data.prefixes[cur_prefix].path);
            printf("    Binary: %s\n", data.prefixes[cur_prefix].binary);
            printf("    Wine: %s\n", data.prefixes[cur_prefix].wine_name);
            printf("    DXVK: %s\n", data.prefixes[cur_prefix].dxvk_name);
            printf("    Arch: %s\n", arch_to_string(data.prefixes[cur_prefix].arch));
            if (cur_prefix < data.prefix_count - 1) printf("\n");
        }
    }
    else {
        printf("    No prefixes found.\n");
    }

    return data.prefix_count;
}

int list_wine_installs() {
    printf("Wine\n");
    if (data.wine_count) {
        for (int cur_wine = 0; cur_wine < data.wine_count; cur_wine++) {
            printf("    Name: %s\n", data.wine_installs[cur_wine].name);
            printf("    Path: %s\n", data.wine_installs[cur_wine].path);
            if (cur_wine < data.wine_count - 1) printf("\n");
        }
    }
    else {
        printf("    No wine installs found.\n");
    }

    return data.wine_count;
}

int list_dxvk_installs() {
    printf("DXVK\n");
    if (data.dxvk_count) {
        for (int cur_dxvk = 0; cur_dxvk < data.dxvk_count; cur_dxvk++) {
            printf("    Name: %s\n", data.dxvk_installs[cur_dxvk].name);
            printf("    Path: %s\n", data.dxvk_installs[cur_dxvk].path);
            if (cur_dxvk < data.dxvk_count - 1) printf("\n");
        }
    }
    else {
        printf("    No DXVK installs found.\n");
    }

    return data.dxvk_count;
}

/**
 * @return 0 if the command completed succesfully, -1 otherwise
 */
int command_list(char *argv[], int argc, int args_index) {
    if (args_index >= argc) {
        list_prefixes();
        printf("\n");
        list_wine_installs();
        printf("\n");
        list_dxvk_installs();
    }
    else {
        for (; args_index < argc; args_index++) {
            if (strcmp(argv[args_index], "prefix") == 0)
                list_prefixes();
            else if (strcmp(argv[args_index], "wine") == 0)
                list_wine_installs();
            else if (strcmp(argv[args_index], "dxvk") == 0)
                list_dxvk_installs();
            else {
                printf("%s: unrecognized component type '%s'. Should be one of 'prefix', 'wine' or 'dxvk'\n", PROGRAM_NAME, argv[args_index]);
                return -1;
            }
        }
    }

    return 0;
}
