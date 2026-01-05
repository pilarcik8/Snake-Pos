#include "game.h"
#include <pthread.h>
#include <stdio.h>

void game_init(game_t *game) {
  game->elapsed_time_sec = 0;
  game->empty_time_ms = 0;
  game->ms_accum = 0;
  game->running = true;
  game->paused = false;
}

void game_toggle_pause(game_t *game) {
  game->paused = !game->paused;
}

int game_update(game_t *game, int player_count, int delta_ms) {
  if (!game->running) return 0;

  /* ----- ŠTANDARTNY REŽIM ----- */
  if (game->mode == GAME_STANDARD) {
    if (player_count == 0) {
      game->empty_time_ms += delta_ms;
      if (game->empty_time_ms >= EMPTY_GAME_TIMEOUT_SEC * 1000) {
        printf("[GAME] No players for %d sec -> game over\n", EMPTY_GAME_TIMEOUT_SEC);
        game->running = false;
        return 0;

      } else {
        int left_time = EMPTY_GAME_TIMEOUT_SEC - game->empty_time_ms / 1000;
        printf("[GAME] If nobody joins, game ends in %d sec\n", left_time);
      }
    } else {
      game->empty_time_ms = 0;
    }  
  }

  // počas pauzy čas neplynie
  if (game->paused) return 1;

  // Čas plynie po sekundách z tickov
  game->ms_accum += delta_ms;
  while (game->ms_accum >= 1000) {
    game->ms_accum -= 1000;
    game->elapsed_time_sec++;

    /* ----- ČASOVÝ REŽIM ----- */
    if (game->mode == GAME_TIMED && game->elapsed_time_sec >= game->time_limit_sec) {
      printf("[GAME] Time limit reached (%d sec)\n", game->time_limit_sec);
      game->running = false;
      return 0;      
    }
  }

  return game->running;
}

void game_end(game_t *game) {
  (void)game; 
}


