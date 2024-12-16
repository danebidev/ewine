#include "remove.h"

#include <string.h>

#include "config.h"
#include "data.h"
#include "util.h"

int remove_prefix(char* prefix_name) {
    int index = -1;

    for (int i = 0; i < data.prefix_count; i++) {
        if (strcmp(data.prefixes[i].name, prefix_name) == 0) {
            index = i;
        }
    }

    if (index == -1) {
        LOG(LOG_ERROR, "prefix not found\n");
        return -1;
    }

    char* prefix_path = strdup(data.prefixes[index].path);

    data.prefix_count--;
    free_prefix(index);
    for (int i = index; i < data.prefix_count; i++) {
        data.prefixes[i] = data.prefixes[i + 1];
    }

    if (data.prefix_count != 0) alloc_component_array(TYPE_PREFIX, data.prefix_count);

    printf("Prefix '%s' has been removed\n", prefix_name);
    // I currently don't plan to automatically remove the prefix directory and let the user handle that.
    // This mainly because I don't want a prefix removal to be a destructive operation.
    // If I'm ever gonna have the program remove the directory, it's
    // going to be either with a big warning and requiring user
    // confirmation, or with a flag you have to pass to the program
    LOG(LOG_WARNING, "NOTE: the prefix directory at '%s' hasn't been deleted. You should delete this manually.\n", prefix_path);

    return 0;
}

int remove_wine(char* wine_name) {
    int index = -1;

    for (int i = 0; i < data.wine_count; i++) {
        if (strcmp(data.wine_installs[i].name, wine_name) == 0) {
            index = i;
        }
    }

    if (index == -1) {
        LOG(LOG_ERROR, "wine not found\n");
        return -1;
    }

    data.wine_count--;
    free_wine(index);
    for (int i = index; i < data.wine_count; i++) {
        data.wine_installs[i] = data.wine_installs[i + 1];
    }

    if (data.wine_count != 0) alloc_component_array(TYPE_WINE, data.wine_count);

    printf("Wine '%s' has been removed\n", wine_name);
    // Wine installs are managed by the user so there's no real
    // need to remove their directory. That's why no warning here

    return 0;
}

int remove_dxvk(char* dxvk_name) {
    int index = -1;

    for (int i = 0; i < data.dxvk_count; i++) {
        if (strcmp(data.dxvk_installs[i].name, dxvk_name) == 0) {
            index = i;
        }
    }

    if (index == -1) {
        LOG(LOG_ERROR, "dxvk not found\n");
        return -1;
    }

    data.dxvk_count--;
    free_dxvk(index);
    for (int i = index; i < data.dxvk_count; i++) {
        data.dxvk_installs[i] = data.dxvk_installs[i + 1];
    }

    if (data.dxvk_count != 0) alloc_component_array(TYPE_DXVK, data.dxvk_count);

    printf("DXVK '%s' has been removed\n", dxvk_name);

    return 0;
}
