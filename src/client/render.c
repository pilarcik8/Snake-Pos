#include "render.h"
#include <stdio.h>

void *render_thread(void *arg) {
    (void)arg;
    printf("Render thread running\n");
    return NULL;
}

