#include <build_config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "data.h"
#include "list.h"
#include "run.h"

void cleanup() {
    config_free();
    data_free();
}

void usage() {
    printf("Usage: %s [options] <command> ...\n", PROGRAM_NAME);
    printf("Commands:\n");
    printf("    list [component]        Lists all components (prefix, wine, dxvk)\n");
    printf("                            Specify the component type to only list those\n");
    printf("    run <prefix>            Runs the specified prefix with the saved settings\n");

    printf("Options:\n");
    printf("    -v                      Display verbose information\n");
    printf("    -h                      Display this help message\n");
}

void parse_argv(char* argv[], int argc, int cur_index) {
    int ret = 0;
    if (cur_index >= argc)
        usage();
    else if (strcmp(argv[cur_index], "run") == 0)
        ret = command_run(argv, argc, cur_index + 1);
    else if (strcmp(argv[cur_index], "list") == 0)
        ret = command_list(argv, argc, cur_index + 1);
    else
        usage();

    if (ret == -1) printf("Try %s -h for more information.\n", PROGRAM_NAME);
}

int main(int argc, char* argv[]) {
    int opt;

    config_init();

    while ((opt = getopt(argc, argv, "vh")) != -1) {
        switch (opt) {
            case 'v':
                config.verbose = 1;
                break;
            case 'h':
                usage();
                exit(0);
                break;
            case '?':
                printf("Try '%s -h' for more information", PROGRAM_NAME);
                exit(1);
        }
    }

    data_init();
    check_data();

    parse_argv(argv, argc, optind);

    cleanup();
    return 0;
}
