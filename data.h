#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TYPE_PREFIX = 0,
    TYPE_WINE = 1,
    TYPE_DXVK = 2,
    TYPE_INVALID
} install_type_t;

typedef enum {
    ARCH_WIN64 = 0,
    ARCH_WIN32 = 1,
    ARCH_INVALID = 255
} arch_type_t;

typedef struct {
    char* name;
    char* path;
} wine_t;

typedef struct {
    char* name;
    char* path;
} dxvk_t;

typedef struct {
    char* name;
    char* path;
    char* binary;
    arch_type_t arch;

    // We keep both component names and later resolve them to component references.
    // I don't really like this method but changing it would require too much work.
    // Ideally we would be resolving wine/dxvk names when we get them from the json.
    char* wine_name;
    wine_t* wine;

    char* dxvk_name;
    dxvk_t* dxvk;
} prefix_t;

typedef struct {
    int prefix_count;
    prefix_t* prefixes;

    int wine_count;
    wine_t* wine_installs;

    int dxvk_count;
    dxvk_t* dxvk_installs;
} data_t;

extern data_t data;

arch_type_t str_to_arch(const char* arch_str);
// Get an arch string from the arch_type_t (an int)
char* arch_to_string(arch_type_t arch);

int alloc_component_array(uint8_t type, size_t length);

void data_init();
int save_data();
void check_data();

void free_prefix(int index);
void free_wine(int index);
void free_dxvk(int index);
void data_free();
