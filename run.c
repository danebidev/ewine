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

static const char* const DXVK_DLLS[] = {
    "d3d8",
    "d3d9",
    "d3d10core",
    "d3d11",
    "dxgi",
};

static const size_t DXVK_DLLS_COUNT = sizeof(DXVK_DLLS) / sizeof(DXVK_DLLS[0]);

int dxvk_remove_entries(prefix_t* prefix) {
    char reg_file_path[PATH_MAX];
    snprintf(reg_file_path, sizeof(reg_file_path), "%s/user.reg", prefix->path);

    // Compiler complains without the + 4
    char tmp_file_path[PATH_MAX + 4];
    snprintf(tmp_file_path, sizeof(tmp_file_path), "%s.tmp", reg_file_path);

    FILE* reg_file = fopen(reg_file_path, "r");
    if (!reg_file) {
        LOG(LOG_ERROR, "Failed to open %s\n", reg_file_path);
        return -1;
    }

    FILE* tmp_file = fopen(tmp_file_path, "w");
    if (!tmp_file) {
        fclose(reg_file);
        LOG(LOG_ERROR, "Failed to open %s\n", tmp_file_path);
        return -1;
    }

    char line[512];
    int in_dll_overrides = 0;

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
        for (size_t i = 0; i < DXVK_DLLS_COUNT; i++) {
            snprintf(pattern, sizeof(pattern), "\"%s\"=", DXVK_DLLS[i]);
            if (strstr(line, pattern) != NULL) {
                found = 1;
                break;
            }
        }

        if (found) continue;

        fputs(line, tmp_file);
    }

    if (reg_file) fclose(reg_file);
    if (tmp_file) fclose(tmp_file);

    if (rename(tmp_file_path, reg_file_path) == -1) {
        LOG(LOG_ERROR, "failed updating registry file at %s\n", reg_file_path);
        return 1;
    }

    return 0;
}

// This just ignores if a dll doesn't exist, since it
// could be missing because of an old wine/dxvk version.
int dxvk_remove_files(prefix_t* prefix) {
    char full_path[PATH_MAX];

    for (int i = 0; i < 2; i++) {
        if (i == 1 && prefix->arch == ARCH_WIN32) break;
        char* dirname;
        if (prefix->arch == ARCH_WIN64)
            dirname = i == 0 ? "system32" : "syswow64";
        else
            dirname = "system32";

        for (size_t j = 0; j < DXVK_DLLS_COUNT; j++) {
            snprintf(full_path, sizeof(full_path), "%s/drive_c/windows/%s/%s.dll", prefix->path, dirname, DXVK_DLLS[j]);
            if (access(full_path, F_OK) == 0) {
                LOG(LOG_DEBUG, "Removing %s\n", full_path);
                if (remove(full_path) != 0) {
                    LOG(LOG_ERROR, "Failed to remove %s\n", full_path);
                    return -1;
                }
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

    int wineboot_pid = fork();
    if (wineboot_pid == -1) {
        LOG(LOG_ERROR, "failed to run wineboot\n");
        return -1;
    }

    if (wineboot_pid == 0) {
        char wineboot_path[PATH_MAX];
        snprintf(wineboot_path, sizeof(wineboot_path), "%s/wineboot", prefix->wine->path);

        setenv("WINEPREFIX", prefix->path, 1);
        execl(wineboot_path, "wineboot", "-u", NULL);
    }

    return 0;
}

int dxvk_add_entries(prefix_t* prefix) {
    char reg_path[PATH_MAX];
    char tmp_reg_path[PATH_MAX];

    snprintf(reg_path, sizeof(reg_path), "%s/user.reg", prefix->path);
    snprintf(tmp_reg_path, sizeof(tmp_reg_path), "%s/user.reg.tmp", prefix->path);

    FILE* reg = fopen(reg_path, "r");
    if (!reg) {
        LOG(LOG_ERROR, "Failed to open %s\n", reg_path);
        return -1;
    }

    FILE* reg_tmp = fopen(tmp_reg_path, "w");
    if (!reg_tmp) {
        fclose(reg);
        LOG(LOG_ERROR, "Failed to open %s\n", tmp_reg_path);
        return -1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), reg)) {
        fputs(line, reg_tmp);

        if (strstr(line, "[Software\\\\Wine\\\\DllOverrides]") != NULL) {
            for (size_t i = 0; i < DXVK_DLLS_COUNT; i++) {
                char dll_override[30];
                snprintf(dll_override, sizeof(dll_override), "\"%s\"=\"native,builtin\"\n", DXVK_DLLS[i]);
                fputs(dll_override, reg_tmp);
            }
        }
    }

    if (reg) fclose(reg);
    if (reg_tmp) fclose(reg_tmp);

    rename(tmp_reg_path, reg_path);

    return 0;
}

int dxvk_copy_dlls(prefix_t* prefix) {
    char command[PATH_MAX * 2 + 9];

    for (int i = 0; i < 2; i++) {
        if (i == 1 && prefix->arch == ARCH_WIN32) break;
        char* windows_subdir = i == 0 ? "system32" : "syswow64";
        char* dxvk_subdir;
        if (prefix->arch == ARCH_WIN64)
            dxvk_subdir = i == 0 ? "x64" : "x32";
        else
            dxvk_subdir = "x32";

        snprintf(command, sizeof(command), "cp \"%s/%s\"/* \"%s/drive_c/windows/%s\"", prefix->dxvk->path, dxvk_subdir, prefix->path, windows_subdir);

        LOG(LOG_DEBUG, "Running command: %s\n", command);
        if (system(command) != 0) return -1;
    }

    return 0;
}

int dxvk_add(prefix_t* prefix) {
    if (dxvk_add_entries(prefix) == -1) {
        LOG(LOG_ERROR, "failed adding DXVK entries to prefix registry\n");
        return -1;
    }

    if (dxvk_copy_dlls(prefix) == -1) {
        LOG(LOG_ERROR, "failed copying DXVK dlls to prefix\n");
        return -1;
    }

    return 0;
}

int check_reg_entry(char* entry) {
    for (size_t i = 0; i < DXVK_DLLS_COUNT; i++) {
        char pattern[30];

        snprintf(pattern, sizeof(pattern), "\"%s\"=\"native\"", DXVK_DLLS[i]);
        LOG(LOG_DEBUG, "Searching for pattern '%s' in user.reg\n", pattern);
        if (strstr(entry, pattern)) {
            LOG(LOG_DEBUG, "Found dll overrride '%s'\n", DXVK_DLLS[i]);
            return 1;
        }

        snprintf(pattern, sizeof(pattern), "\"%s\"=\"native,builtin\"", DXVK_DLLS[i]);
        LOG(LOG_DEBUG, "Searching for pattern '%s' in user.reg\n", pattern);
        if (strstr(entry, pattern)) {
            LOG(LOG_DEBUG, "Found dll overrride '%s'\n", DXVK_DLLS[i]);
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
                if (line[0] == '[') goto end;
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
        if (prefix->dxvk) return 0;
        LOG(LOG_DEBUG, "Removing DXVK from prefix\n");
        if (dxvk_remove(prefix) == -1) {
            LOG(LOG_ERROR, "DXVK couldn't be removed\n");
            return -1;
        }
    }
    else {
        if (!prefix->dxvk) return 0;
        LOG(LOG_DEBUG, "Adding DXVK to prefix\n");
        if (dxvk_add(prefix) == -1) {
            LOG(LOG_ERROR, "DXVK couldn't be applied\n");
            return -1;
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
        LOG(LOG_ERROR, "failed to run wine\n");
        return -1;
    }

    if (wine_pid == 0) {
        char wine_path[PATH_MAX];
        char* execname = prefix->arch == ARCH_WIN64 ? "wine64" : "wine";
        snprintf(wine_path, sizeof(wine_path), "%s/%s", prefix->wine->path, execname);

        setenv("WINEPREFIX", prefix->path, 0);

        // For when i don't want wine to spam my output
        // Remember to change before commit
        // execl("exit", "exit", "0", NULL);
        // exit(0);
        execl(wine_path, execname, prefix->binary, NULL);
    }

    return 0;
}

int command_run(char** argv, int argc, int args_index) {
    if (args_index == argc) {
        printf("%s: missing prefix name.\n", PROGRAM_NAME);
        return -1;
    }
    if (args_index + 1 != argc) {
        printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index + 1]);
        return -1;
    }

    char* prefix_name = argv[args_index];
    prefix_t* prefix = get_prefix(prefix_name);
    if (!prefix) {
        LOG(LOG_ERROR, "no prefix named '%s'\n", prefix_name);
        return 1;
    }

    if (run(prefix) == -1) {
        LOG(LOG_ERROR, "can't run prefix '%s'\n", prefix_name);
        return 1;
    }

    return 0;
}
