#include "data.h"

#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "util.h"

#define BASE_CONFIG_FILE \
    "{\n\
    \"prefix\": [],\n\
    \"wine\": [],\n\
    \"dxvk\": []\n\
}"

void create_data_file() {
    char mkdir_command[PATH_SIZE + 9];

    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir -p %s", config.data_file);
    remove_last_path_component(mkdir_command);

    // Using system() invokes the shell so it's slower
    // than just implementing it in pure C, but I'm lazy
    system(mkdir_command);
    printf("Data file not found at %s. Creating it.\n", config.data_file);

    FILE* file;
    if ((file = fopen(config.data_file, "w")) != NULL)
        // Is saving the base file in a define even a good idea? idk, it works
        fprintf(file, BASE_CONFIG_FILE);
    else {
        printf("error: can't write to %s\n", config.data_file);
        exit(1);
    }
}

void parse_data() {
    char* json_text = read_file(config.data_file);
    cJSON* json = cJSON_Parse(json_text);
    free(json_text);

    // TODO

    cJSON_Delete(json);
}

void data_init() {
    if (access(config.data_file, F_OK) != -1)
        parse_data();
    else
        create_data_file();
}

void data_create_pfx(char* name) {
}

uint8_t data_remove_pfx(char* name) {
    return 1;
}
