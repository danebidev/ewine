#include <build_config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "data.h"

void cleanup() {
    config_free();
    data_free();
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
                printf("Try '%s --help' for more information", PROGRAM_NAME);
                exit(1);
        }
    }

    cleanup();
    return 0;
}
