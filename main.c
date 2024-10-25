#include <getopt.h>
#include <unistd.h>

#include "config.h"
#include "data.h"

void cleanup() {
    config_free();
}

int main(int argc, char *argv[]) {
    config_init();
    data_init();

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

    cleanup();
    return 0;
}
