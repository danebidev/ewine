#include "data.h"

#include <cjson/cJSON.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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
char* arch_to_string(arch_type_t arch) {
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
        LOG(LOG_ERROR, "can't create directory %s", path);
    }
    free(path);

    LOG(LOG_DEBUG, "Data file not found at %s - creating it.\n", config.data_file);

    FILE* file;
    if ((file = fopen(config.data_file, "w")) != NULL)
        // Is saving the default config in a define even a good idea? idk, it works
        fprintf(file, BASE_CONFIG_FILE);
    else {
        LOG(LOG_ERROR, "can't write to %s\n", config.data_file);
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
    if (!cJSON_IsString(item)) LOG(LOG_ERROR, "field %s is not a string", key);

    char* result = strdup(item->valuestring);
    if (!result) {
        LOG(LOG_ERROR, "memory allocation failed for string %s", key);
    }

    return result;
}

void update_prefix_wine_references() {
    for (int i = 0; i < data.prefix_count; i++) {
        if (data.prefixes[i].wine_name) {
            data.prefixes[i].wine = NULL;
            for (int j = 0; j < data.wine_count; j++) {
                if (strcmp(data.wine_installs[j].name, data.prefixes[i].wine_name) == 0) {
                    data.prefixes[i].wine = &data.wine_installs[j];
                    break;
                }
            }
        }
    }
}

void update_prefix_dxvk_references() {
    for (int i = 0; i < data.prefix_count; i++) {
        data.prefixes[i].dxvk = NULL;
        if (data.prefixes[i].dxvk_name && strcmp(data.prefixes[i].dxvk_name, "none") != 0) {
            for (int j = 0; j < data.dxvk_count; j++) {
                if (strcmp(data.dxvk_installs[j].name, data.prefixes[i].dxvk_name) == 0) {
                    data.prefixes[i].dxvk = &data.dxvk_installs[j];
                    break;
                }
            }
        }
    }
}

/**
 * Allocates or reallocates memory for an array of the specified type.
 * @param type The type of array to allocate (TYPE_PREFIX, TYPE_WINE, or TYPE_DXVK)
 * @param length The number of elements to allocate
 */
int alloc_component_array(uint8_t type, size_t length) {
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
            LOG(LOG_ERROR, "invalid type");
            exit(1);
    }

    void* new_ptr;
    if (!*target)
        new_ptr = calloc(length, size);
    else
        new_ptr = reallocarray(*target, length, size);

    if (!new_ptr) return -1;

    *target = new_ptr;

    if (type == TYPE_WINE)
        update_prefix_wine_references();
    else if (type == TYPE_DXVK)
        update_prefix_dxvk_references();

    return 0;
}

/**
 * Parses a componenet of the specified type and stores it in the right array, at the specified index
 * @param type The type of component to parse (TYPE_PREFIX, TYPE_WINE, or TYPE_DXVK)
 * @param array_index The index of the right array where the component will be stored
 * @param json The json object to parse
 * @return 0 if the object was parsed correctly, -1 otherwise
 */
int parse_component(install_type_t type, size_t array_index, cJSON* json) {
    char* componentstr;
    switch (type) {
        case TYPE_PREFIX:
            componentstr = "prefix";
            break;
        case TYPE_WINE:
            componentstr = "wine install";
            break;
        case TYPE_DXVK:
            componentstr = "DXVK install";
            break;
        case TYPE_INVALID:
            LOG(LOG_ERROR, "Invalid type. Should be impossible. Please report this.");
    }

    if (!cJSON_IsObject(json)) {
        LOG(LOG_WARNING, "A %s is not an object - ignoring it\n", componentstr);
        return -1;
    }

    char* name = json_get_string(json, "name");
    if (!name || name[0] == '\0') {
        if (!name)
            LOG(LOG_WARNING, "A %s is missing the name field - ignoring it\n", componentstr);
        else
            LOG(LOG_WARNING, "A %s has an empty name field - ignoring it\n", componentstr);
        return -1;
    }

    char* path = json_get_string(json, "path");
    if (!path || path[0] == '\0') {
        if (!path)
            LOG(LOG_WARNING, "%s %s is missing the path field - ignoring it\n", componentstr, name);
        else
            LOG(LOG_WARNING, "%s %s has an empty path field - ignoring it\n", componentstr, name);

        free(name);
        return -1;
    }

    switch (type) {
        case TYPE_PREFIX:
            data.prefixes[array_index].name = name;
            data.prefixes[array_index].path = path;
            data.prefixes[array_index].binary = json_get_string(json, "binary");
            data.prefixes[array_index].wine_name = json_get_string(json, "wine");
            data.prefixes[array_index].dxvk_name = json_get_string(json, "dxvk");

            char* arch = json_get_string(json, "arch");
            data.prefixes[array_index].arch = str_to_arch(arch);
            if (arch) free(arch);
            break;
        case TYPE_WINE:
            data.wine_installs[array_index].name = name;
            data.wine_installs[array_index].path = path;
            break;
        case TYPE_DXVK:
            data.dxvk_installs[array_index].name = name;
            data.dxvk_installs[array_index].path = path;
            break;
        case TYPE_INVALID:
            LOG(LOG_ERROR, "Invalid type. Should be impossible. Please report this.");
    }

    return 0;
}

/**
 * Parses an array of items from the JSON configuration file.
 * Handles prefix, Wine, and DXVK installations.
 * @param json JSON array to parse
 * @param type Type of data to parse (TYPE_PREFIX, TYPE_WINE, or TYPE_DXVK)
 */
void parse_component_array(cJSON* json, install_type_t type) {
    if (!cJSON_IsArray(json))
        LOG(LOG_ERROR, "invalid data file, %s is supposed to be an array\n", component_type_to_string(type));

    size_t count = cJSON_GetArraySize(json);
    if (count) {
        if (alloc_component_array(type, count) != 0)
            LOG(LOG_ERROR, "memory allocation failed for %s array\n", component_type_to_string(type));
    }

    size_t cur_element = 0;
    cJSON* element = NULL;

    cJSON_ArrayForEach(element, json) {
        if (parse_component(type, cur_element, element) == 0)
            cur_element++;
    }

    if (cur_element < count && cur_element) {
        if (alloc_component_array(type, cur_element) != 0)
            LOG(LOG_ERROR, "memory allocation failed for %s array\n", component_type_to_string(type));
    }

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
        case TYPE_INVALID:
            LOG(LOG_ERROR, "Invalid type. Should be impossible. Please report this.");
            break;
    }
}

/**
 * Parses the entire data file and populates the global data structure.
 * Reads and parses the JSON configuration file, handling prefix, Wine,
 * and DXVK installation data.
 */
void parse_data() {
    LOG(LOG_DEBUG, "Parsing data file: %s\n", config.data_file);
    char* json_text = read_file(config.data_file);
    if (!json_text) {
        LOG(LOG_ERROR, "couldn't read data file at %s", config.data_file);
    }

    cJSON* json = cJSON_Parse(json_text);
    free(json_text);

    if (!json) LOG(LOG_ERROR, "can't parse %s\n", config.data_file);

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

int create_prefix_json(cJSON* prefixes) {
    for (int i = 0; i < data.prefix_count; i++) {
        cJSON* prefix = cJSON_CreateObject();

        cJSON_AddStringToObject(prefix, "name", data.prefixes[i].name);
        cJSON_AddStringToObject(prefix, "path", data.prefixes[i].path);
        cJSON_AddStringToObject(prefix, "binary", data.prefixes[i].binary);

        if (data.prefixes[i].wine)
            cJSON_AddStringToObject(prefix, "wine", data.prefixes[i].wine->name);
        else
            cJSON_AddStringToObject(prefix, "wine", "none");

        if (data.prefixes[i].dxvk)
            cJSON_AddStringToObject(prefix, "dxvk", data.prefixes[i].dxvk->name);
        else
            cJSON_AddStringToObject(prefix, "dxvk", "none");

        cJSON_AddStringToObject(prefix, "arch", arch_to_string(data.prefixes[i].arch));

        cJSON_AddItemToArray(prefixes, prefix);
    }

    return 0;
}

int create_wine_json(cJSON* wines) {
    for (int i = 0; i < data.wine_count; i++) {
        cJSON* wine = cJSON_CreateObject();

        cJSON_AddStringToObject(wine, "name", data.wine_installs[i].name);
        cJSON_AddStringToObject(wine, "path", data.wine_installs[i].path);

        cJSON_AddItemToArray(wines, wine);
    }

    return 0;
}

int create_dxvk_json(cJSON* dxvk_installs) {
    for (int i = 0; i < data.dxvk_count; i++) {
        cJSON* dxvk = cJSON_CreateObject();

        cJSON_AddStringToObject(dxvk, "name", data.dxvk_installs[i].name);
        cJSON_AddStringToObject(dxvk, "path", data.dxvk_installs[i].path);

        cJSON_AddItemToArray(dxvk_installs, dxvk);
    }

    return 0;
}

int save_data() {
    cJSON* json = cJSON_CreateObject();
    int err = -1;

    cJSON* prefix = NULL;
    cJSON* wine = NULL;
    cJSON* dxvk = NULL;

    if ((prefix = cJSON_AddArrayToObject(json, "prefix")) == NULL) goto end;
    if ((wine = cJSON_AddArrayToObject(json, "wine")) == NULL) goto end;
    if ((dxvk = cJSON_AddArrayToObject(json, "dxvk")) == NULL) goto end;

    if (create_prefix_json(prefix) == -1) goto end;
    if (create_wine_json(wine) == -1) goto end;
    if (create_dxvk_json(dxvk) == -1) goto end;

    char* c = cJSON_Print(json);

    FILE* config_file = fopen(config.data_file, "w");
    if (!config_file) {
        return -1;
    }

    fwrite(c, sizeof(char), strlen(c), config_file);
    free(c);
    fclose(config_file);

    err = 0;
end:
    cJSON_Delete(json);
    return err;
}

int check_file_perms(char* dir_path, char* name, int perm) {
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/%s", dir_path, name);

    int result = access(path, perm);

    return result;
}

void check_wine(wine_t* wine) {
    if (check_file_perms(wine->path, "wine", X_OK))
        LOG(LOG_WARNING, "'wine' binary in wine '%s' is not executable\n", wine->name);

    if (check_file_perms(wine->path, "wine64", X_OK))
        LOG(LOG_WARNING, "'wine64' binary in wine '%s' is not executable\n", wine->name);

    if (check_file_perms(wine->path, "wineboot", X_OK))
        LOG(LOG_WARNING, "'wineboot' binary in wine '%s' is not executable\n", wine->name);
}

void check_dxvk(dxvk_t* dxvk) {
    if (check_file_perms(dxvk->path, "x64", R_OK))
        LOG(LOG_WARNING, "'x64' directory in dxvk '%s' is not accessible\n", dxvk->name);

    if (check_file_perms(dxvk->path, "x32", R_OK))
        LOG(LOG_WARNING, "'x32' directory in dxvk '%s' is not accessible\n", dxvk->name);
}

void check_prefix(prefix_t* prefix) {
    if (access(prefix->path, R_OK) == -1)
        LOG(LOG_WARNING, "Can't write to %s for prefix '%s'\n", prefix->path, prefix->name);

    if (access(prefix->binary, R_OK) == -1)
        LOG(LOG_WARNING, "Can't read %s for prefix '%s'\n", prefix->binary, prefix->name);

    if (prefix->wine_name != NULL) {
        wine_t* wine = NULL;

        if (strcmp(prefix->wine_name, "none") == 0) {
            LOG(LOG_WARNING, "Prefix %s doesn't have a wine version set\n", prefix->name);
            wine = NULL;
            goto end_wine;
        }

        for (int i = 0; i < data.wine_count; i++) {
            if (strcmp(data.wine_installs[i].name, prefix->wine_name) == 0) {
                wine = &data.wine_installs[i];
                break;
            }
        }

        if (!wine) {
            LOG(LOG_WARNING, "Prefix %s has an invalid wine version '%s'", prefix->name, prefix->wine_name);
            if (data.wine_count) {
                LOG(LOG_WARNING, " - setting to '%s'\n", data.wine_installs[0].name);
                wine = &data.wine_installs[0];
            }
            else {
                LOG(LOG_WARNING, " and no wine installs were found - unsetting wine version for this prefix.\n");
                wine = NULL;
            }
        }

    end_wine:
        prefix->wine = wine;
    }

    if (prefix->dxvk_name != NULL) {
        dxvk_t* dxvk = NULL;

        if (strcmp(prefix->dxvk_name, "none") == 0) {
            LOG(LOG_WARNING, "Prefix %s doesn't have a DXVK version set\n", prefix->name);
            dxvk = NULL;
            goto end_dxvk;
        }

        for (int i = 0; i < data.dxvk_count; i++) {
            if (strcmp(data.dxvk_installs[i].name, prefix->dxvk_name) == 0) {
                dxvk = &data.dxvk_installs[i];
                break;
            }
        }

        if (!dxvk) {
            LOG(LOG_WARNING, "Prefix %s has an invalid DXVK version '%s'", prefix->name, prefix->dxvk_name);
            if (data.dxvk_count) {
                LOG(LOG_WARNING, " - setting to '%s'\n", data.dxvk_installs[0].name);
                dxvk = &data.dxvk_installs[0];
            }
            else {
                LOG(LOG_WARNING, " and no DXVK installs were found - unsetting DXVK version for this prefix.\n");
                dxvk = NULL;
            }
        }

    end_dxvk:
        prefix->dxvk = dxvk;
    }

    if (prefix->arch == ARCH_INVALID) {
        LOG(LOG_WARNING, "Prefix %s has an invalid wine prefix arch - setting to 'win64'\n", prefix->name);
        prefix->arch = ARCH_WIN64;
    }
}

void check_data() {
    for (int i = 0; i < data.wine_count; i++) {
        check_wine(&data.wine_installs[i]);
    }
    for (int i = 0; i < data.dxvk_count; i++) {
        check_dxvk(&data.dxvk_installs[i]);
    }
    for (int i = 0; i < data.prefix_count; i++) {
        check_prefix(&data.prefixes[i]);
    }
}

void free_prefix(int index) {
    free(data.prefixes[index].name);
    free(data.prefixes[index].path);
    free(data.prefixes[index].binary);
    free(data.prefixes[index].wine_name);
    free(data.prefixes[index].dxvk_name);
}

void free_wine(int index) {
    free(data.wine_installs[index].name);
    free(data.wine_installs[index].path);
}

void free_dxvk(int index) {
    free(data.dxvk_installs[index].name);
    free(data.dxvk_installs[index].path);
}

/**
 * Frees all allocated memory used by the data subsystem.
 */
void data_free() {
    for (int i = 0; i < data.prefix_count; i++) {
        free_prefix(i);
    }
    free(data.prefixes);

    for (int i = 0; i < data.wine_count; i++) {
        free_wine(i);
    }
    free(data.wine_installs);

    for (int i = 0; i < data.dxvk_count; i++) {
        free_dxvk(i);
    }
    free(data.dxvk_installs);
}
