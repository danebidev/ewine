#include "create.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#include "build_config.h"
#include "config.h"
#include "data.h"
#include "util.h"

wine_t* get_wine(char* wine_name) {
    if (!wine_name) return NULL;

    for (int i = 0; i < data.wine_count; i++) {
        if (strcmp(data.wine_installs[i].name, wine_name) == 0) return &data.wine_installs[i];
    }

    return NULL;
}

dxvk_t* get_dxvk(char* dxvk_name) {
    if (!dxvk_name) return NULL;

    for (int i = 0; i < data.dxvk_count; i++) {
        if (strcmp(data.dxvk_installs[i].name, dxvk_name) == 0) return &data.dxvk_installs[i];
    }

    return NULL;
}

int wineboot_prefix(char* prefix_path, wine_t* wine) {
    int pid = fork();

    if (pid == -1) {
        LOG(LOG_ERROR, "fork() failed: %s\n", strerror(errno));
        return -1;
    }

    if (pid == 0) {
        char wineboot_path[PATH_MAX];
        snprintf(wineboot_path, sizeof(wineboot_path), "%s/wineboot", wine->path);

        setenv("WINEPREFIX", prefix_path, 1);
        execl(wineboot_path, "wineboot", "-i", NULL);

        LOG(LOG_ERROR, "execl failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    wait(NULL);

    return 0;
}

void read_wine(wine_t** wine) {
    char wine_name[64];

    if (data.wine_count == 0) {
        LOG(LOG_WARNING, "No Wine installs found - skipping\n");
        *wine = NULL;
        return;
    }

    if (data.wine_count == 1) {
        printf("Only one version of Wine is availabe - setting Wine for this prefix to '%s'\n", data.wine_installs[0].name);
        *wine = &data.wine_installs[0];
        return;
    }

    while (1) {
        while (read_string_input("Wine", NULL, wine_name, sizeof(wine_name)) == -1) {
            printf("Invalid string. Retry.\n");
        }

        *wine = get_wine(wine_name);
        if (*wine) break;

        printf("Invalid Wine version. Available versions:\n");
        for (int i = 0; i < data.wine_count; i++) {
            printf("- %s\n", data.wine_installs[i].name);
        }
    }

    return;
}

void read_dxvk(dxvk_t** dxvk) {
    char dxvk_name[64];

    if (data.dxvk_count == 0) {
        LOG(LOG_WARNING, "No DXVK installs found - skipping\n");
        *dxvk = NULL;
        return;
    }

    if (data.dxvk_count == 1) {
        printf("Only one version of DXVK is availabe - setting DXVK for this prefix to '%s'\n", data.dxvk_installs[0].name);
        *dxvk = &data.dxvk_installs[0];
        return;
    }

    while (1) {
        while (read_string_input("DXVK (empty for none)", NULL, dxvk_name, sizeof(dxvk_name)) == -1) {
            printf("Invalid string. Retry.\n");
        }

        if (dxvk_name[0] == 0) {
            *dxvk = NULL;
            break;
        }

        *dxvk = get_dxvk(dxvk_name);
        if (*dxvk) break;

        printf("Invalid DXVK version. Available versions:\n");
        for (int i = 0; i < data.dxvk_count; i++) {
            printf("- %s\n", data.dxvk_installs[i].name);
        }
    }

    return;
}

int create_prefix(char* prefix_name, char* prefix_path, char* binary_path, char* arch, wine_t* wine, dxvk_t* dxvk) {
    prefix_t prefix = {
        .name = strdup(prefix_name),
        .path = strdup(prefix_path),
        .binary = strdup(binary_path),
        .arch = str_to_arch(arch),
        .wine = wine,
        .dxvk = dxvk,
        .wine_name = NULL,
        .dxvk_name = NULL
    };

    data.prefix_count++;

    if (alloc_component_array(TYPE_PREFIX, data.prefix_count) != 0) {
        LOG(LOG_ERROR, "memory allocation failed for prefix array\n");
        return -1;
    }

    data.prefixes[data.prefix_count - 1] = prefix;

    printf("Creating prefix...\n");

    if (mkdirp(prefix_path) != 0) {
        LOG(LOG_ERROR, "failed creating prefix directory\n");
        return -1;
    }

    if (!wine) {
        printf("Not initializing the prefix because no wine was set.\n");
    }
    else if (wineboot_prefix(prefix_path, wine) == -1) {
        LOG(LOG_ERROR, "failed creating the prefix\n");
        return -1;
    }

    printf("Prefix created.\n");

    return 0;
}

int read_and_create_prefix() {
    char prefix_name[64];
    char prefix_path[PATH_MAX];
    char binary_path[PATH_MAX];
    char arch[10];
    wine_t* wine = NULL;
    dxvk_t* dxvk = NULL;

    while (read_string_input("Prefix name", NULL, prefix_name, sizeof(prefix_name)) == -1 || prefix_name[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    char default_value[PATH_MAX + 71];  // So the compiler doesn't scream at me
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

    read_wine(&wine);
    read_dxvk(&dxvk);

    while (read_string_input("Arch", "win64", arch, sizeof(arch)) == -1) {
        printf("Invalid string. Retry.\n");
    }

    if (arch[0] == '\0') {
        strcpy(arch, "win64");
    }

    wordexp_t p;
    wordexp(prefix_path, &p, 0);
    strcpy(prefix_path, *p.we_wordv);

    if (prefix_path[0] != '/') {
        LOG(LOG_ERROR, "the prefix path has to be absolute\n");
        wordfree(&p);
        return -1;
    }

    wordexp(binary_path, &p, WRDE_REUSE);
    strcpy(binary_path, *p.we_wordv);

    if (binary_path[0] != '/') {
        LOG(LOG_ERROR, "the binary path has to be absolute\n");
        wordfree(&p);
        return -1;
    }

    wordfree(&p);

    return create_prefix(prefix_name, prefix_path, binary_path, arch, wine, dxvk);
}

int create_wine() {
    char wine_name[64];
    char wine_path[PATH_MAX];

    while (read_string_input("Wine name", NULL, wine_name, sizeof(wine_name)) == -1 || wine_name[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    char default_value[PATH_MAX + 71];  // So the compiler doesn't scream at me
    snprintf(default_value, sizeof(default_value), "%s/wine/%s", config.data_dir, wine_name);

    while (read_string_input("Wine path", NULL, wine_path, sizeof(wine_path)) == -1 || wine_path[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    wordexp_t p;
    wordexp(wine_path, &p, 0);
    strcpy(wine_path, *p.we_wordv);

    wordfree(&p);

    if (wine_path[0] != '/') {
        LOG(LOG_ERROR, "the wine path has to be absolute\n");
        return -1;
    }

    wine_t wine = {
        .name = strdup(wine_name),
        .path = strdup(wine_path),
    };

    data.wine_count++;

    if (alloc_component_array(TYPE_WINE, data.wine_count) != 0) {
        LOG(LOG_ERROR, "memory allocation failed for wine array\n");
        return -1;
    }

    data.wine_installs[data.wine_count - 1] = wine;

    printf("Prefix created.\n");

    return 0;
}

int create_dxvk() {
    char dxvk_name[64];
    char dxvk_path[PATH_MAX];

    while (read_string_input("DXVK name", NULL, dxvk_name, sizeof(dxvk_name)) == -1 || dxvk_name[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    char default_value[PATH_MAX + 71];  // So the compiler doesn't scream at me
    snprintf(default_value, sizeof(default_value), "%s/dxvk/%s", config.data_dir, dxvk_name);

    while (read_string_input("DXVK path", NULL, dxvk_path, sizeof(dxvk_path)) == -1 || dxvk_path[0] == '\0') {
        printf("Invalid or empty string. Retry.\n");
    }

    wordexp_t p;
    wordexp(dxvk_path, &p, 0);
    strcpy(dxvk_path, *p.we_wordv);

    wordfree(&p);

    if (dxvk_path[0] != '/') {
        LOG(LOG_ERROR, "the DXVK path has to be absolute\n");
        return -1;
    }

    dxvk_t dxvk = {
        .name = strdup(dxvk_name),
        .path = strdup(dxvk_path),
    };

    data.dxvk_count++;

    if (alloc_component_array(TYPE_DXVK, data.dxvk_count) != 0) {
        LOG(LOG_ERROR, "memory allocation failed for DXVK array\n");
        return -1;
    }

    data.dxvk_installs[data.dxvk_count - 1] = dxvk;

    printf("DXVK created.\n");

    return 0;
}

/**
 * @return 0 if the command completed succesfully, -1 otherwise
 */
int command_create(char* argv[], int argc, int args_index) {
    if (args_index + 1 < argc) {
        printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }

    if (strcmp(argv[args_index], "prefix") == 0) {
        if (read_and_create_prefix() == -1) {
            LOG(LOG_ERROR, "can't create prefix\n");
            return 1;
        }
    }
    else if (strcmp(argv[args_index], "wine") == 0) {
        if (create_wine() == -1) {
            LOG(LOG_ERROR, "can't create wine\n");
            return 1;
        }
    }
    else if (strcmp(argv[args_index], "dxvk") == 0) {
        if (create_dxvk() == -1) {
            LOG(LOG_ERROR, "can't create dxvk\n");
            return 1;
        }
    }
    else {
        printf("%s: unrecognized component type '%s'. Should be one of 'prefix', 'wine' or 'dxvk'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }

    save_data();
    return 0;
}
