#include "game.h"
#include <pthread.h>
#include <stdio.h>

void game_init(game_t *game) {
  game->elapsed_time = 0;
  game->empty_time_ms = 0;
  game->ms_accum = 0;
  game->running = 1;
  game->paused = false;
  pthread_mutex_init(&game->lock, NULL);
}

void game_toggle_pause(game_t *game) {
  pthread_mutex_lock(&game->lock);
  game->paused = !game->paused;
  pthread_mutex_unlock(&game->lock);
}

int game_update(game_t *game, int player_count, int delta_ms) {
  pthread_mutex_lock(&game->lock);

  if (!game->running) {
    pthread_mutex_unlock(&game->lock);
    return 0;
  }

  if (player_count == 0) {
    game->empty_time_ms += delta_ms;
    if (game->empty_time_ms >= EMPTY_GAME_TIMEOUT_SEC * 1000) {
      printf("[GAME] No players for %d sec -> game over\n", EMPTY_GAME_TIMEOUT_SEC);
      game->running = 0;
      pthread_mutex_unlock(&game->lock);
      return 0;

    } else {
      int left_time = EMPTY_GAME_TIMEOUT_SEC - game->empty_time_ms / 1000;
      printf("[GAME] If nobody joins, game ends in %d sec\n", left_time);
    }
  } else {
    game->empty_time_ms = 0;
  }  

  // Počas pauzy čas NEPLYNE a nič sa neodpočítava
  if (game->paused) {
    pthread_mutex_unlock(&game->lock);
    return 1;
  }

  /* nazbieraj milisekundy a spracuj celé sekundy */
  game->ms_accum += delta_ms;

  while (game->ms_accum >= 1000) {
    game->ms_accum -= 1000;
    game->elapsed_time += 1;

    /* ----- ČASOVÝ REŽIM ----- */
    if (game->mode == GAME_TIMED) {
      if (game->elapsed_time >= game->time_limit) { 
        printf("[GAME] Time limit reached (%d sec)\n", game->time_limit);
        game->running = 0;
        break;
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


