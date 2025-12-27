#include "snake.h"

void snake_init(snake_t *snake, int player_id, position_t start) {
    snake->player_id = player_id;
    snake->length = 1;
    snake->body[0] = start;
    snake->direction = RIGHT;
    snake->alive = true;
    snake->score = 0;
}

void snake_move(snake_t *snake) {
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }

    switch (snake->direction) {
        case UP:    snake->body[0].y--; break;
        case DOWN:  snake->body[0].y++; break;
        case LEFT:  snake->body[0].x--; break;
        case RIGHT: snake->body[0].x++; break;
    }
}

bool snake_self_collision(snake_t *snake) {
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[0].x == snake->body[i].x &&
            snake->body[0].y == snake->body[i].y)
            return true;
    }
    return false;
}

