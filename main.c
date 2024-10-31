#include <build_config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "data.h"
#include "list.h"

void cleanup() {
    config_free();
    data_free();
}

void usage() {
    printf("Usage: %s [options] <command> ...\n", PROGRAM_NAME);
    printf("Commands:\n");
    printf("    list [component]        Lists all components (prefix, wine, dxvk)\n");
    printf("                            Specify the component type to only list those\n");
}

void parse_argv(char* argv[], int argc, int cur_index) {
    int ret = 0;
    if (cur_index >= argc)
        usage();
    else if (strcmp(argv[cur_index++], "list") == 0)
        ret = command_list(argv, argc, cur_index);
    else
        usage();

    if (ret == -1) printf("Try %s -h for more information", PROGRAM_NAME);
}

int main(int argc, char* argv[]) {
    config_init();
    data_init();

    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                config.verbose = 1;
                break;
            case '?':
                printf("Try '%s -h' for more information", PROGRAM_NAME);
                exit(1);
        }
    }

    parse_argv(argv, argc, optind);

    cleanup();
    return 0;
}
