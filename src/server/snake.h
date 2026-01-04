#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>
#include "../common/types.h"
#include "../common/config.h"

typedef struct {
    int player_id;
    position_t body[MAX_SNAKE_LENGTH];
    int length;
    direction_t direction;
    bool alive;
    int score;
    bool dir_locked;
} snake_t;

void snake_init(snake_t *s, int player_id, position_t start);
void snake_set_direction(snake_t *s, direction_t dir);
position_t snake_next_head(const snake_t *s);
void snake_move(snake_t *s, int world_w, int world_h, bool wrap);
void snake_grow(snake_t *s);
bool snake_self_collision(const snake_t *s);

#endif

