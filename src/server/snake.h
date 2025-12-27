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
} snake_t;

void snake_init(snake_t *snake, int player_id, position_t start);
void snake_move(snake_t *snake);
bool snake_self_collision(snake_t *snake);

#endif

