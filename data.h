#pragma once

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
    char* wine;
    char* dxvk;
    arch_type_t arch;
} prefix_t;

typedef struct {
    char* name;
    char* path;
} wine_t;

typedef struct {
    char* name;
    char* path;
} dxvk_t;

typedef struct {
    int prefix_count;
    prefix_t* prefixes;

    int wine_count;
    wine_t* wine_installs;

    int dxvk_count;
    dxvk_t* dxvk_installs;
} data_t;

extern data_t data;

// Get an arch string from the arch_type_t (an int)
char* archstr(arch_type_t arch);

void data_init();
void data_free();
