#include <config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char *data_file;
    char *prefix_dir;
} config_t;

config_t config;

void cleanup(int exit_code) {
    free(config.data_file);
    free(config.prefix_dir);
}

void config_init() {
    const char *xdg_data_home = getenv("XDG_DATA_HOME");
    const char *home = getenv("HOME");

    config.data_file = malloc(512);
    config.prefix_dir = malloc(512);

    // If XDG_DATA_HOME is defined use that as default data directory
    if (xdg_data_home) {
        snprintf(config.data_file, 512, "%s/%s/config.json", xdg_data_home, PROGRAM_NAME);
        snprintf(config.prefix_dir, 512, "%s/%s/prefix", xdg_data_home, PROGRAM_NAME);
    }
    // Fallback to HOME/.local/share/abc if XDG_DATA_HOME is not set
    else if (home) {
        snprintf(default_path, sizeof(default_path), "%s/.local/share/abc", home);
    }
    else {
        // If both XDG_DATA_HOME and HOME are not defined
        fprintf(stderr, "Error: Neither XDG_DATA_HOME nor HOME are defined.\n");
        return 1;
    }
}

void config_load() {
}

int main(int argc, char *argv[]) {
    config_init();

    /*
    int opt;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case '?':
                printf("Try '%s --help' for more information", PROGRAM_NAME);
                die();
        }
    }
    */
}
