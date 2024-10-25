#include "config.h"

#include <build_config.h>
#include <stdio.h>
#include <stdlib.h>

config_t config;

void config_init() {
    const char *xdg_data_home = getenv("XDG_DATA_HOME");
    const char *home = getenv("HOME");

    // If XDG_DATA_HOME is defined use that as default data directory
    if (xdg_data_home) {
        snprintf(config.data_file, PATH_SIZE, "%s/%s/config.json", xdg_data_home, PROGRAM_NAME);
        snprintf(config.prefix_dir, PATH_SIZE, "%s/%s/prefix", xdg_data_home, PROGRAM_NAME);
    }
    // Fallback to HOME/.local/share/abc if XDG_DATA_HOME is not set
    else if (home) {
        snprintf(config.data_file, PATH_SIZE, "%s/%s/config.json", home, PROGRAM_NAME);
        snprintf(config.prefix_dir, PATH_SIZE, "%s/%s/prefix", home, PROGRAM_NAME);
    }
    else {
        // If both XDG_DATA_HOME and HOME are not defined
        fprintf(stderr, "error: either HOME or XDG_DATA_HOME have to be defined.\n");
        exit(1);
    }
}

void config_free() {}
