#ifndef GAME_H
#define GAME_H

#include "../common/types.h"

#include <pthread.h>
#include <stdbool.h>

typedef struct {
  game_mode_t mode;     // GAME_STANDARD / GAME_TIMED
  int time_limit_sec;   // pre GAME_TIMED

  int elapsed_time_sec; // od štartu hry
  int ms_accum;         // nazbierane ms pre prechod na 1 sekundu
  int empty_time_ms;    // ms bez hráčov - GAME_TIMED
    
  bool allowed_multiplayer;

  bool running;          
} game_t;

void game_init(game_t *game);

int game_update(game_t *game, int player_count, int delta_ms);

void game_toggle_pause(game_t *game);

#endif

