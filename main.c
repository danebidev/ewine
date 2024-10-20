#include <config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    char *config_file;
    char *data_dir;
} config_t;

config_t config;

void die() {
    exit(1);
}

void config_init() {
    config.config_file = "${XDG_CONFIG_HOME}/ewine/config.toml";
    config.data_dir = "${XDG_DATA_HOME}/ewine";
}

void config_load() {
}

int main(int argc, char *argv[]) {
    config_init();

    int opt;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case '?':
                printf("Try '%s --help' for more information", PROGRAM_NAME);
                die();
        }
    }

    if (access(config.config_file, R_OK) != -1) {
        config_load();
    }
}
