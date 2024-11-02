#pragma once

#define LOG(LOG_LEVEL, __fmt, ...)                            \
    do {                                                      \
        if (LOG_LEVEL == LOG_DEBUG && !config.verbose) break; \
        switch (LOG_LEVEL) {                                  \
            case LOG_ERROR:                                   \
                printf("\e[38;5;1m");                         \
                printf("error: " __fmt, ##__VA_ARGS__);       \
                break;                                        \
            case LOG_WARNING:                                 \
                printf("\e[38;5;3m");                         \
                printf(__fmt, ##__VA_ARGS__);                 \
                break;                                        \
            case LOG_DEBUG:                                   \
                printf("\e[38;5;0m");                         \
                printf(__fmt, ##__VA_ARGS__);                 \
                break;                                        \
        }                                                     \
        printf("\e[0m");                                      \
    }                                                         \
    while (0)

typedef enum {
    LOG_ERROR = 0,
    LOG_WARNING = 1,
    LOG_DEBUG = 2
} log_level_t;

void remove_last_path_component(const char* path);

char* read_file(const char* path);

int mkdirp(const char* path);
