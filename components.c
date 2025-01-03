#include <components.h>
#include <stdio.h>
#include <string.h>

#include "build_config.h"
#include "config.h"
#include "create.h"
#include "data.h"
#include "remove.h"
#include "util.h"

int command_create(char* argv[], int argc, int args_index) {
    if (args_index < argc) {
        printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }

    if (read_and_create_prefix() == -1) {
        LOG(LOG_ERROR, "can't create prefix\n");
        return 1;
    }

    save_data();
    return 0;
}

int command_remove(char* argv[], int argc, int args_index) {
    if (args_index + 1 < argc) {
        printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }
    else if (args_index >= argc) {
        printf("%s: missing prefix name\n", PROGRAM_NAME);
        return -1;
    }

    if (remove_prefix(argv[args_index]) == -1) {
        LOG(LOG_ERROR, "can't remove prefix\n");
        return 1;
    }

    save_data();
    return 0;
}

int command_wine(char* argv[], int argc, int args_index) {
    if (args_index >= argc) {
        printf("%s: missing operation type. Should be either 'add' or 'rem'\n", PROGRAM_NAME);
        return -1;
    }

    if (strcmp(argv[args_index], "add") == 0) {
        if (args_index + 1 < argc) {
            printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index]);
            return -1;
        }

        if (create_wine() == -1) {
            LOG(LOG_ERROR, "can't create wine\n");
            return 1;
        }
    }
    else if (strcmp(argv[args_index], "rem") == 0) {
        if (args_index + 2 < argc) {
            printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index + 2]);
            return -1;
        }
        else if (args_index + 1 >= argc) {
            printf("%s: missing wine name\n", PROGRAM_NAME);
            return -1;
        }

        if (remove_wine(argv[args_index + 1]) == -1) {
            LOG(LOG_ERROR, "can't remove wine\n");
            return 1;
        }
    }
    else {
        printf("%s: unrecognized operation '%s'. Should be either 'add' or 'rem'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }

    save_data();
    return 0;
}

int command_dxvk(char* argv[], int argc, int args_index) {
    if (args_index >= argc) {
        printf("%s: missing operation type. Should be either 'add' or 'rem'\n", PROGRAM_NAME);
        return -1;
    }

    if (strcmp(argv[args_index], "add") == 0) {
        if (args_index + 1 < argc) {
            printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index]);
            return -1;
        }

        if (create_dxvk() == -1) {
            LOG(LOG_ERROR, "can't create dxvk\n");
            return 1;
        }
    }
    else if (strcmp(argv[args_index], "rem") == 0) {
        if (args_index + 2 < argc) {
            printf("%s: unexpected argument '%s'\n", PROGRAM_NAME, argv[args_index + 2]);
            return -1;
        }
        else if (args_index + 1 >= argc) {
            printf("%s: missing dxvk name\n", PROGRAM_NAME);
            return -1;
        }

        if (remove_dxvk(argv[args_index + 1]) == -1) {
            LOG(LOG_ERROR, "can't remove dxvk\n");
            return 1;
        }
    }
    else {
        printf("%s: unrecognized operation '%s'. Should be either 'add' or 'rem'\n", PROGRAM_NAME, argv[args_index]);
        return -1;
    }

    save_data();
    return 0;
}
