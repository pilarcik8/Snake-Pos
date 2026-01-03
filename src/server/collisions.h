#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <stdbool.h>
#include "world.h"
#include "snake.h"
#include "../common/config.h"

typedef struct {
    bool hit_wall;
    bool hit_self;
    bool hit_other;
} collision_result_t;

collision_result_t collisions_check_one(const world_t *w, const snake_t *snakes, int snake_count, int idx);

#endif

