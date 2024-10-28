#pragma once

#define LOG(LOG_LEVEL, __fmt, ...)                      \
    do {                                                \
        if (LOG_LEVEL != LOG_DEBUG || config.verbose) { \
            switch (LOG_LEVEL) {                        \
                case LOG_ERROR:                         \
                    printf("\e[38;5;1m==>\e[0m ");      \
                    break;                              \
                case LOG_WARNING:                       \
                    printf("\e[38;5;3m==>\e[0m ");      \
                    break;                              \
                case LOG_INFO:                          \
                    printf("\e[38;5;2m==>\e[0m ");      \
                    break;                              \
                case LOG_DEBUG:                         \
                    printf("\e[38;5;0m");               \
                    break;                              \
            }                                           \
            printf(__fmt, ##__VA_ARGS__);               \
            printf("\e[0m");                            \
        }                                               \
    }                                                   \
    while (0)

#define ERROR(__fmt, ...)                               \
    do {                                                \
        LOG(LOG_ERROR, "error: " __fmt, ##__VA_ARGS__); \
        exit(1);                                        \
    }                                                   \
    while (0)

typedef enum {
    LOG_ERROR = 0,
    LOG_WARNING = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3
} log_level_t;

void remove_last_path_component(const char* path);

char* read_file(const char* path);

int mkdirp(const char* path);
