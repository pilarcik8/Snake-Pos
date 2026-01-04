#include "game.h"
#include <pthread.h>
#include <stdio.h>

void game_init(game_t *game) {
    game->elapsed_time = 0;
    game->empty_time = 0;
    game->ms_accum = 0;
    game->running = 1;
    pthread_mutex_init(&game->lock, NULL);
}

int game_update(game_t *game, int player_count) {
    pthread_mutex_lock(&game->lock);

    if (!game->running) {
        pthread_mutex_unlock(&game->lock);
        return 0;
    }

    // ----- ČASOVÝ REŽIM -----
    if (game->mode == GAME_TIMED) {
        if (game->elapsed_time >= game->time_limit) {
            printf("[GAME] Time limit reached (%d sec)\n", game->time_limit);
            game->running = 0;
        }
    }

    // ----- ŠTANDARDNÝ REŽIM -----
    if (game->mode == GAME_STANDARD) {
        static int empty_time = 0;

        if (player_count == 0) {
            empty_time++;
            if (empty_time >= EMPTY_GAME_TIMEOUT_SEC) {
                printf("[GAME] No players for %d sec -> game over\n",
                       EMPTY_GAME_TIMEOUT_SEC);
                game->running = 0;
            }
        } else {
            empty_time = 0;
        }
    }

    int still_running = game->running;
    pthread_mutex_unlock(&game->lock);
    return still_running;
}

void game_end(game_t *game) {
    pthread_mutex_destroy(&game->lock);
}

