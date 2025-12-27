#include "game.h"
#include <stdio.h>

void game_init(game_t *game) {
    game->running = 1;
    game->elapsed_time = 0;
    pthread_mutex_init(&game->lock, NULL);
    printf("Game initialized\n");
}

void game_update(game_t *game) {
    pthread_mutex_lock(&game->lock);
    game->elapsed_time++;
    pthread_mutex_unlock(&game->lock);
}

void game_end(game_t *game) {
    pthread_mutex_destroy(&game->lock);
    game->running = 0;
    printf("Game ended\n");
}

