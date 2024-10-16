#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    char *program_name;
    char *config_file;
    char *data_dir;
} config_t;

config_t config;

void die() {
    exit(1);
}

void config_init() {
    config.program_name = "ewine";
    config.config_file = "${XDG_CONFIG_HOME}/ewine/config.toml";
    config.data_dir = "${XDG_DATA_HOME}/ewine";
}

int main(int argc, char *argv[]) {
    config_init();

    int opt;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                config.config_file = optarg;
                break;
            case '?':
                // printf("Try '%s --help' for more information", config.program_name);
                die();
        }
    }

    if (access(config.config_file, R_OK) != -1) {
        printf("config file not found. Please create it");
    }
}
