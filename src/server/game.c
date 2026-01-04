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

int game_update(game_t *game, int player_count, int delta_ms) {
    pthread_mutex_lock(&game->lock);

    if (!game->running) {
        pthread_mutex_unlock(&game->lock);
        return 0;
    }

    game->ms_accum += delta_ms;
    while (game->ms_accum >= 1000) {
        game->ms_accum -= 1000;
        game->elapsed_time += 1;

        if (game->mode == GAME_TIMED) {
            if (game->elapsed_time >= game->time_limit) {
                printf("[GAME] Time limit reached (%d sec)\n", game->time_limit);
                game->running = 0;
                break;
            }
        }

        if (game->mode == GAME_STANDARD) {
            if (player_count == 0) {
                game->empty_time++;
                if (game->empty_time >= EMPTY_GAME_TIMEOUT_SEC) {
                    printf("[GAME] No players for %d sec -> game over\n", EMPTY_GAME_TIMEOUT_SEC);
                    game->running = 0;
                    pthread_mutex_unlock(&game->lock);
                    return 0;
                }
            } else {
                game->empty_time = 0;
            }
        }
    }

    int still_running = game->running;
    pthread_mutex_unlock(&game->lock);
    return still_running;
}

void game_end(game_t *game) {
    pthread_mutex_destroy(&game->lock);
}

