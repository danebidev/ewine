#include "data.h"

#include <cjson/cJSON.h>
#include <stdint.h>
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

data_t data;

/**
 * Converts an architecture string to its corresponding enumeration value.
 * @param arch_str Architecture string ("win64" or "win32")
 * @return ARCH_WIN64, ARCH_WIN32, or ARCH_INVALID
 */
arch_type_t str_to_arch(const char* arch_str) {
    if (!arch_str) return ARCH_INVALID;
    if (strcmp(arch_str, "win64") == 0) return ARCH_WIN64;
    if (strcmp(arch_str, "win32") == 0) return ARCH_WIN32;
    return ARCH_INVALID;
}

/**
 * Converts an architecture enumeration value to its string representation.
 * @param arch Architecture enumeration value
 * @return "win64", "win32", or NULL if invalid
 */
char* archstr(arch_type_t arch) {
    if (arch == ARCH_WIN64) return "win64";
    if (arch == ARCH_WIN32) return "win32";
    return NULL;
}

/**
 * Converts a type enumeration value to its string representation.
 * @param type Type enumeration value (TYPE_PREFIX, TYPE_WINE, or TYPE_DXVK)
 * @return String representation of the type or "invalid" if unknown
 */
char* component_type_to_string(uint8_t type) {
    switch (type) {
        case TYPE_PREFIX:
            return "prefix";
        case TYPE_WINE:
            return "wine";
        case TYPE_DXVK:
            return "dxvk";
        default:
            return "invalid";
    }
}

/**
 * Creates a new data file with default configuration if it doesn't exist.
 * Creates necessary parent directories and writes the base config JSON structure.
 * Exits if unable to write to the file.
 */
void create_data_file() {
    char* path = strdup(config.data_file);
    remove_last_path_component(path);

    if (mkdirp(path) == -1) {
        ERROR("can't create directory %s", path);
    }
    free(path);

    LOG(LOG_INFO, "Data file not found at %s - creating it.\n", config.data_file);

    FILE* file;
    if ((file = fopen(config.data_file, "w")) != NULL)
        // Is saving the default config in a define even a good idea? idk, it works
        fprintf(file, BASE_CONFIG_FILE);
    else {
        ERROR("can't write to %s\n", config.data_file);
    }
}

/**
 * Retrieves a string value from a cJSON object for the given key.
 * @param obj The cJSON object to search in
 * @param key The key to look up
 * @return Newly allocated string containing the value, or NULL if not found/invalid
 */
char* json_get_string(cJSON* obj, char* key) {
    cJSON* item = cJSON_GetObjectItem(obj, key);

    if (!item) return NULL;
    if (!cJSON_IsString(item)) ERROR("field %s is not a string", key);

    char* result = strdup(item->valuestring);
    if (!result) {
        ERROR("memory allocation failed for string %s", key);
    }

    return result;
}

/**
 * Allocates or reallocates memory for an array of the specified type.
 * @param type The type of array to allocate (TYPE_PREFIX, TYPE_WINE, or TYPE_DXVK)
 * @param length The number of elements to allocate
 */
void alloc_component_array(uint8_t type, size_t length) {
    void** target;
    size_t size = 0;

    switch (type) {
        case TYPE_PREFIX:
            target = (void**)&data.prefixes;
            size = sizeof(prefix_t);
            break;
        case TYPE_WINE:
            target = (void**)&data.wine_installs;
            size = sizeof(wine_t);
            break;
        case TYPE_DXVK:
            target = (void**)&data.dxvk_installs;
            size = sizeof(dxvk_t);
            break;
        default:
            ERROR("invalid type");
    }

    void* new_ptr;
    if (!*target)
        new_ptr = malloc(length * size);
    else
        new_ptr = reallocarray(*target, length, size);

    if (!new_ptr) ERROR("memory allocation failed for %s array\n", component_type_to_string(type));

    *target = new_ptr;
}

/**
 * Parses a prefix entry from JSON and stores it in the data structure.
 * @param prefix_index Index in the prefixes array to store the parsed data
 * @param json JSON object containing prefix data
 * @return 0 on success, -1 on failure
 */
int parse_prefix(size_t prefix_index, cJSON* json) {
    prefix_t* current = &data.prefixes[prefix_index];

    current->name = json_get_string(json, "name");
    if (!current->name) {
        LOG(LOG_WARNING, "A prefix is missing the name field - ignoring it\n");
        return -1;
    }

    current->path = json_get_string(json, "path");
    if (!current->path) {
        LOG(LOG_WARNING, "Prefix %s is missing the path field - ignoring it\n", current->name);
        free(current->name);
        return -1;
    }

    current->wine = json_get_string(json, "wine");
    current->dxvk = json_get_string(json, "dxvk");

    char* arch = json_get_string(json, "arch");
    current->arch = str_to_arch(arch);
    if (arch) free(arch);

    return 0;
}

/**
 * Parses a Wine installation entry from JSON and stores it in the data structure.
 * @param wine_index Index in the wine_installs array to store the parsed data
 * @param json JSON object containing Wine installation data
 * @return 0 on success, -1 on failure
 */
int parse_wine(size_t wine_index, cJSON* json) {
    wine_t* current = &data.wine_installs[wine_index];

    current->name = json_get_string(json, "name");
    if (!current->name) {
        LOG(LOG_WARNING, "A wine install is missing the name field - ignoring it\n");
        return -1;
    }

    current->path = json_get_string(json, "path");
    if (!current->path) {
        LOG(LOG_WARNING, "Wine %s is missing the path field - ignoring it\n", current->name);
        free(current->name);
        return -1;
    }

    return 0;
}

/**
 * Parses a DXVK installation entry from JSON and stores it in the data structure.
 * @param dxvk_index Index in the dxvk_installs array to store the parsed data
 * @param json JSON object containing DXVK installation data
 * @return 0 on success, -1 on failure
 */
int parse_dxvk(size_t dxvk_index, cJSON* json) {
    dxvk_t* current = &data.dxvk_installs[dxvk_index];

    current->name = json_get_string(json, "name");
    if (!current->name) {
        LOG(LOG_WARNING, "A DXVK install is missing the name field - ignoring it\n");
        return -1;
    }

    current->path = json_get_string(json, "path");
    if (!current->path) {
        LOG(LOG_WARNING, "DXVK %s is missing the path field - ignoring it\n", current->name);
        free(current->name);
        return -1;
    }

    return 0;
}

/**
 * Parses an array of items from the JSON configuration file.
 * Handles prefix, Wine, and DXVK installations.
 * @param json JSON array to parse
 * @param type Type of data to parse (TYPE_PREFIX, TYPE_WINE, or TYPE_DXVK)
 */
void parse_component_array(cJSON* json, uint8_t type) {
    if (!cJSON_IsArray(json))
        ERROR("invalid data file, %s is supposed to be an array\n", component_type_to_string(type));

    size_t count = cJSON_GetArraySize(json);
    alloc_component_array(type, count);

    size_t cur_element = 0;
    cJSON* element = NULL;

    cJSON_ArrayForEach(element, json) {
        if (!cJSON_IsObject(element)) {
            continue;
        }

        int (*parse_func)(size_t, cJSON*) = NULL;
        switch (type) {
            case TYPE_PREFIX:
                parse_func = parse_prefix;
                break;
            case TYPE_WINE:
                parse_func = parse_wine;
                break;
            case TYPE_DXVK:
                parse_func = parse_dxvk;
                break;
        }

        if (parse_func(cur_element, element) == 0)
            cur_element++;
    }

    if (cur_element < count) alloc_component_array(type, cur_element);

    switch (type) {
        case TYPE_PREFIX:
            data.prefix_count = cur_element;
            break;
        case TYPE_WINE:
            data.wine_count = cur_element;
            break;
        case TYPE_DXVK:
            data.dxvk_count = cur_element;
            break;
    }
}

/**
 * Parses the entire data file and populates the global data structure.
 * Reads and parses the JSON configuration file, handling prefix, Wine,
 * and DXVK installation data.
 */
void parse_data() {
    LOG(LOG_INFO, "Parsing data file: %s\n", config.data_file);
    char* json_text = read_file(config.data_file);

    cJSON* json = cJSON_Parse(json_text);
    free(json_text);

    if (!json) ERROR("can't parse %s\n", config.data_file);

    data.prefixes = NULL;
    data.wine_installs = NULL;
    data.dxvk_installs = NULL;

    cJSON* prefixes = cJSON_GetObjectItem(json, "prefix");
    cJSON* wine = cJSON_GetObjectItem(json, "wine");
    cJSON* dxvk = cJSON_GetObjectItem(json, "dxvk");

    if (prefixes) parse_component_array(prefixes, TYPE_PREFIX);
    if (wine) parse_component_array(wine, TYPE_WINE);
    if (dxvk) parse_component_array(dxvk, TYPE_DXVK);

    cJSON_Delete(json);
}

/**
 * Initializes the data subsystem by either parsing an existing data file
 * or creating a new one with default configuration if it doesn't exist.
 */
void data_init() {
    if (access(config.data_file, F_OK) != -1)
        parse_data();
    else
        create_data_file();
}

/**
 * Frees all allocated memory used by the data subsystem.
 */
void data_free() {
    for (int i = 0; i < data.prefix_count; i++) {
        free(data.prefixes[i].name);
        free(data.prefixes[i].path);
        free(data.prefixes[i].wine);
        free(data.prefixes[i].dxvk);
    }
    free(data.prefixes);

    for (int i = 0; i < data.wine_count; i++) {
        free(data.wine_installs[i].name);
        free(data.wine_installs[i].path);
    }
    free(data.wine_installs);

    for (int i = 0; i < data.dxvk_count; i++) {
        free(data.dxvk_installs[i].name);
        free(data.dxvk_installs[i].path);
    }
    free(data.dxvk_installs);
}
