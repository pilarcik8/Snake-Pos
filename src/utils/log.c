#include "log.h"
#include <stdio.h>

void log_info(const char *msg) {
    printf("[INFO] %s\n", msg);
}

void log_error(const char *msg) {
    fprintf(stderr, "[ERROR] %s\n", msg);
}

