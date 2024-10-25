#pragma once

#include <stdint.h>

#define ARCH_WIN64 0
#define ARCH_WIN32 1

typedef struct {
    char* name;
    char* path;
    char* wine;
    char* dxvk;
    uint8_t arch;  // 0 for win64 and 1 for win32
} prefix_t;

typedef struct {
    char* name;
    char* wine;
    char* wine64;
    char* wineboot;
} wine_t;

typedef struct {
    char* name;
    char* x64;
    char* x86;
} dxvk_t;

typedef struct {
    prefix_t* prefixes;
    wine_t* wine_installs;
    dxvk_t* dxvk_installs;
} data_t;

void data_init();

void data_create_pfx(char* name);
uint8_t data_remove_pfx(char* name);
