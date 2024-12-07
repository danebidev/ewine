#pragma once

#include <linux/limits.h>
#include <stdbool.h>
typedef struct {
    bool verbose;
    char data_file[PATH_MAX];
    char data_dir[PATH_MAX];
} config_t;

extern config_t config;

void config_init();
void config_free();
