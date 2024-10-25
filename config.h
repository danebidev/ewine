#pragma once

#define PATH_SIZE 512

typedef struct {
    char data_file[PATH_SIZE];
    char prefix_dir[PATH_SIZE];
} config_t;

extern config_t config;

void config_init();
void config_free();
