#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include "../common/types.h"

typedef struct {
    game_mode_t mode;     // GAME_STANDARD / GAME_TIMED
    int time_limit_sec;   // pre GAME_TIMED

    int elapsed_time_sec; // od štartu hry
    int ms_accum;         // nazbierane ms pre prechod na 1 sekundu
    int empty_time_ms;    // ms bez hráčov
    
    bool allowed_multiplayer;

    bool running;          
} game_t;

void game_init(game_t *game);

/**
 * aktualizuje stav hry podľa delta_ms (SERVER_TICK_MS).
 * ret: 1 - pokracuje, 0 - koniec
 */
int game_update(game_t *game, int player_count, int delta_ms);

void game_toggle_pause(game_t *game);

void game_end(game_t *game);

#endif

