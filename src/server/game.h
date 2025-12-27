#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include "world.h"
#include "snake.h"
#include "../common/types.h"
#include "../common/config.h"

typedef struct {
    game_mode_t mode;
    int time_limit;
    int elapsed_time;
    int running;
    pthread_mutex_t lock;
} game_t;

void game_init(game_t *game);
void game_update(game_t *game);
void game_end(game_t *game);

#endif

