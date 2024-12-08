#include "create.h"

#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "build_config.h"
#include "config.h"
#include "data.h"
#include "util.h"

wine_t* get_wine(char* wine_name) {
    for (int i = 0; i < data.wine_count; i++) {
        if (strcmp(data.wine_installs[i].name, wine_name) == 0) return &data.wine_installs[i];
    }

    return NULL;
}

dxvk_t* get_dxvk(char* dxvk_name) {
    for (int i = 0; i < data.dxvk_count; i++) {
        if (strcmp(data.dxvk_installs[i].name, dxvk_name) == 0) return &data.dxvk_installs[i];
    }

    return NULL;
}

int create() {
    // malloc because data_free() assumes all
    // the data is malloced and frees it
    char* prefix_name = malloc(sizeof(char) * 64);
    char* prefix_path = malloc(sizeof(char) * PATH_MAX);
    char* binary_path = malloc(sizeof(char) * PATH_MAX);
    char* arch = malloc(sizeof(char) * 16);
    wine_t* wine = NULL;
    dxvk_t* dxvk = NULL;

    while (read_string_input("Prefix name", NULL, prefix_name, sizeof(prefix_name)) == -1 || prefix_name[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    char default_value[PATH_MAX];
    snprintf(default_value, sizeof(default_value), "%s/prefix/%s", config.data_dir, prefix_name);

    while (read_string_input("Prefix path", default_value, prefix_path, sizeof(prefix_path)) == -1) {
        printf("Invalid string. Retry.\n");
    }

    if (prefix_path[0] == '\0') {
        strcpy(prefix_path, default_value);
    }

    while (read_string_input("Binary path", NULL, binary_path, sizeof(binary_path)) == -1 || binary_path[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    if (data.wine_count == 0) {
        LOG(LOG_WARNING, "No Wine installs found - skipping\n");
    }
    else if (data.wine_count == 1) {
        printf("Only one version of Wine is availabe - setting Wine for this prefix to '%s'\n", data.wine_installs[0].name);
        wine = &data.wine_installs[0];
    }
    else {
        char wine_name[64];
        while (read_string_input("Wine", NULL, wine_name, sizeof(wine_name)) == -1 && (wine = get_wine(wine_name))) {
            printf("Invalid or empty string. Retry.\n");
        }
    }

    if (data.dxvk_count == 0) {
        LOG(LOG_WARNING, "No DXVK installs found - skipping\n");
    }
    else if (data.dxvk_count == 1) {
        printf("Only one version of DXVK is availabe - setting DXVK for this prefix to '%s'\n", data.dxvk_installs[0].name);
        dxvk = &data.dxvk_installs[0];
    }
    else {
        char dxvk_name[64];
        while (read_string_input("DXVK", NULL, dxvk_name, sizeof(dxvk_name)) == -1 && (dxvk = get_dxvk(dxvk_name))) {
            printf("Invalid or empty string. Retry.\n");
        }
    }

    while (read_string_input("Arch", "win64", arch, sizeof(arch)) == -1) {
        printf("Invalid string. Retry.\n");
    }
    if (arch[0] == '\0') {
        free(arch);
        arch = "win64";
    }

    prefix_t prefix;
    prefix.name = prefix_name;
    prefix.path = prefix_path;
    prefix.binary = binary_path;
    prefix.arch = str_to_arch(arch);
    prefix.wine = wine;
    prefix.dxvk = dxvk;
    prefix.wine_name = NULL;
    prefix.dxvk_name = NULL;

    data.prefix_count++;

    if (alloc_component_array(TYPE_PREFIX, data.prefix_count) != 0) {
        LOG(LOG_ERROR, "memory allocation failed for prefix array\n");
        return -1;
    }

    data.prefixes[data.prefix_count - 1] = prefix;

    printf("Prefix created.\n");

    return 0;
}

/**
 * @return 0 if the command completed succesfully, -1 otherwise
 */
int command_create(char* argv[], int argc, int args_index) {
    if (args_index < argc) {
        printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }

    if (create() == -1) {
        LOG(LOG_ERROR, "can't create prefix\n");
        return 1;
    }

    save_data();
    return 0;
}
