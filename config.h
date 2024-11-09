#pragma once

#include <linux/limits.h>
typedef struct {
    _Bool verbose;
    char data_file[PATH_MAX];
    char prefix_dir[PATH_MAX];
} config_t;

extern config_t config;

void config_init();
void config_free();
