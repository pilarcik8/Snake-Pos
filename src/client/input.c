#include "input.h"
#include <stdio.h>

void *input_thread(void *arg) {
    (void)arg;
    printf("Input thread running\n");
    return NULL;
}

