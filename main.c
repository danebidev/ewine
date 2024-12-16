#include <build_config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "components.h"
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
    printf("                            If the component type is not specified, it will list all components\n");
    printf("    run <prefix>            Runs the specified prefix with the saved settings\n");
    printf("    create                  Creates a new prefix and prompts for prefix settings\n");
    printf("    remove <prefix>         Removes the specified prefix\n");
    printf("    wine add                Adds a new wine install\n");
    printf("    wine rem <wine>         Removes the specified wine install\n");
    printf("    dxvk add                Adds a new dxvk install\n");
    printf("    dxvk rem <dxvk>         Removes the specified dxvk install\n");

    printf("\nOptions:\n");
    printf("    -v                      Display verbose information\n");
    printf("    -h                      Display this help message\n");
}

void run_ewine(char* argv[], int argc, int cur_index) {
    int ret = 0;
    if (cur_index >= argc)
        usage();
    else if (strcmp(argv[cur_index], "run") == 0)
        ret = command_run(argv, argc, cur_index + 1);
    else if (strcmp(argv[cur_index], "list") == 0)
        ret = command_list(argv, argc, cur_index + 1);
    else if (strcmp(argv[cur_index], "create") == 0)
        ret = command_create(argv, argc, cur_index + 1);
    else if (strcmp(argv[cur_index], "remove") == 0)
        ret = command_remove(argv, argc, cur_index + 1);
    else if (strcmp(argv[cur_index], "wine") == 0)
        ret = command_wine(argv, argc, cur_index + 1);
    else if (strcmp(argv[cur_index], "dxvk") == 0)
        ret = command_dxvk(argv, argc, cur_index + 1);
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
            case '?':
                printf("Try '%s -h' for more information", PROGRAM_NAME);
                exit(1);
        }
    }

    data_init();
    check_data();

    run_ewine(argv, argc, optind);

    cleanup();
    return 0;
}
