#ifndef WORLD_H
#define WORLD_H

#include "../common/types.h"
#include "../common/config.h"

typedef struct {
    int width;
    int height;
    cell_t cells[MAX_WORLD_HEIGHT][MAX_WORLD_WIDTH];
} world_t;

void world_generate(world_t *world);
bool world_is_free(world_t *world, int x, int y);

#endif

