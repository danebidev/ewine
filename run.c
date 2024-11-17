#include "run.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "build_config.h"
#include "config.h"
#include "data.h"
#include "util.h"

int dxvk_remove_entries(prefix_t* prefix) {
    char reg_file_path[PATH_MAX];
    snprintf(reg_file_path, sizeof(reg_file_path), "%s/user.reg", prefix->path);

    char tmp_file_path[PATH_MAX];
    snprintf(tmp_file_path, sizeof(tmp_file_path) + 4, "%s.tmp", reg_file_path);

    FILE* reg_file = fopen(reg_file_path, "r");
    FILE* tmp_file = fopen(tmp_file_path, "w");

    char line[512];
    int in_dll_overrides = 0;
    char* dll_overrides[] = { "d3d8", "d3d9", "d3d10core", "d3d11", "dxgi" };

    while (fgets(line, sizeof(line), reg_file)) {
        if (strstr(line, "[Software\\\\Wine\\\\DllOverrides]") != NULL) {
            in_dll_overrides = 1;
            fputs(line, tmp_file);
            continue;
        }

        if (!in_dll_overrides) {
            fputs(line, tmp_file);
            continue;
        }

        if (line[0] == '[' || line[0] == '\n' || line[0] == '\r') {
            in_dll_overrides = 0;
            fputs(line, tmp_file);
            continue;
        }

        char pattern[50];

        int found = 0;
        for (size_t i = 0; i < sizeof(dll_overrides) / sizeof(dll_overrides[0]); i++) {
            snprintf(pattern, sizeof(pattern), "\"%s\"=", dll_overrides[i]);
            if (strstr(line, pattern) != NULL) {
                found = 1;
                break;
            }
        }

        if (found) continue;

        fputs(line, tmp_file);
    }

    if (rename(tmp_file_path, reg_file_path) == -1) {
        LOG(LOG_ERROR, "updating registry file at %s failed\n", reg_file_path);
        remove(tmp_file_path);
        return 1;
    }

    fclose(reg_file);
    fclose(tmp_file);

    return 0;
}

// This just ignores if a dll doesn't exist, since it
// could be missing because of an old wine/dxvk version.
int dxvk_remove_files(prefix_t* prefix) {
    char* dlls[] = {
        "system32/dxgi.dll",
        "system32/d3d8.dll",
        "system32/d3d9.dll",
        "system32/d3d10core.dll",
        "system32/d3d11.dll",

        "syswow64/dxgi.dll",
        "syswow64/d3d8.dll",
        "syswow64/d3d9.dll",
        "syswow64/d3d10core.dll",
        "syswow64/d3d11.dll"
    };

    char full_path[PATH_MAX];
    for (size_t i = 0; i < sizeof(dlls) / sizeof(dlls[0]); i++) {
        snprintf(full_path, sizeof(full_path), "%s/drive_c/windows/%s", prefix->path, dlls[i]);
        if (access(full_path, F_OK) == 0) {
            LOG(LOG_DEBUG, "Removing %s\n", full_path);
            if (remove(full_path) != 0) {
                LOG(LOG_ERROR, "Failed to remove %s\n", full_path);
                return -1;
            }
        }
    }

    return 0;
}

int dxvk_remove(prefix_t* prefix) {
    if (dxvk_remove_entries(prefix) == -1) {
        LOG(LOG_ERROR, "DXVK registry entries couldn't be removed\n");
    }

    if (dxvk_remove_files(prefix) == -1) {
        LOG(LOG_ERROR, "DXVK dlls couldn't be removed\n");
    }

    return 0;
}

int check_reg_entry(char* entry) {
    char* dxvk_registry_entries[] = { "d3d8", "d3d9", "d3d10core", "d3d11", "dxgi" };
    for (size_t i = 0; i < sizeof(dxvk_registry_entries) / sizeof(dxvk_registry_entries[0]); i++) {
        char pattern[30];

        snprintf(pattern, sizeof(pattern), "\"%s\"=\"native\"", dxvk_registry_entries[i]);
        LOG(LOG_DEBUG, "Searching for pattern '%s' in user.reg\n", pattern);
        if (strstr(entry, pattern)) {
            LOG(LOG_DEBUG, "Found dll overrride '%s'\n", dxvk_registry_entries[i]);
            return 1;
        }

        snprintf(pattern, sizeof(pattern), "\"%s\"=\"native,builtin\"", dxvk_registry_entries[i]);
        LOG(LOG_DEBUG, "Searching for pattern '%s' in user.reg\n", pattern);
        if (strstr(entry, pattern)) {
            LOG(LOG_DEBUG, "Found dll overrride '%s'\n", dxvk_registry_entries[i]);
            return 1;
        }
    }

    return 0;
}

int is_dxvk_applied(prefix_t* prefix) {
    char reg_path[PATH_MAX];

    snprintf(reg_path, sizeof(reg_path), "%s/user.reg", prefix->path);

    FILE* reg_file = fopen(reg_path, "r");
    char line[1024];  // Surely no line is longer than 1024 characters
    int dxvk_found = 0;

    while (fgets(line, sizeof(line), reg_file)) {
        if (strstr(line, "[Software\\\\Wine\\\\DllOverrides]") != NULL) {
            while (fgets(line, sizeof(line), reg_file)) {
                // Sections usually end with a blank line so we
                // can use that to check for end of section
                // Shouldn't break if the user manually removes the
                // blank line, it's just gonna continue reading
                if (line[0] == '\n' || line[0] == '\r') break;
                if (check_reg_entry(line)) {
                    dxvk_found = 1;
                    goto end;
                }
            }
        }
    }

end:
    fclose(reg_file);

    return dxvk_found;
}

int apply_dxvk(prefix_t* prefix) {
    if (is_dxvk_applied(prefix)) {
        LOG(LOG_DEBUG, "Removing dxvk from prefix\n");
        if (dxvk_remove(prefix) == -1) {
            LOG(LOG_ERROR, "DXVK couldn't be removed\n");
        }
    }

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
        LOG(LOG_ERROR, "failed to apply DXVK setting\n");
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
        char wine_path[PATH_MAX];
        snprintf(wine_path, sizeof(wine_path), "%s/wine64", prefix->wine->path);

        setenv("WINEPREFIX", prefix->path, 0);

        char* args[] = { "wine64", prefix->binary, NULL };

        // For when i don't want wine to spam my output
        // Remember to change before commit
        // execl("exit", "0", NULL);
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
