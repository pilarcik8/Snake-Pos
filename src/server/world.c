#include "world.h"
#include <stdlib.h>

void world_generate(world_t *world) {
    for (int y = 0; y < world->height; y++) {
        for (int x = 0; x < world->width; x++) {
            world->cells[y][x] = CELL_EMPTY;
        }
    }
}

bool world_is_free(world_t *world, int x, int y) {
    if (x < 0 || y < 0 || x >= world->width || y >= world->height)
        return false;
    return world->cells[y][x] == CELL_EMPTY;
}

