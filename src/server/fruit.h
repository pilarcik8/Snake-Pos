#ifndef FRUIT_H
#define FRUIT_H

#include <stdbool.h>
#include "world.h"
#include "snake.h"
#include "../common/config.h"

typedef struct {
    position_t pos[MAX_PLAYERS];
    int count;
} fruit_state_t;

void fruit_init(fruit_state_t *fs);

void fruit_sync(world_t *w,
                const snake_t *snakes,
                const bool *slot_used,
                int snake_count,
                int active_snakes,
                fruit_state_t *fs);

bool fruit_handle_eating(world_t *w,
                         snake_t *snakes,
                         const bool *slot_used,
                         int snake_count,
                         fruit_state_t *fs);

#endif

