#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include "../common/types.h"
#include "../common/config.h" //pouzite v .c 

typedef struct {
    game_mode_t mode;
    int time_limit;       // len pre GAME_TIMED
    int elapsed_time;     // sekundy od štartu hry
    int running;          // 1 = beží, 0 = skončila
    int ms_accum;         // nazbierane ms pre prechod na 1 sekundu
    int empty_time;       // sekundy bez hráčov
    
    bool paused;          

    pthread_mutex_t lock;
} game_t;

void game_init(game_t *game);

/**
 * Aktualizuje stav hry podľa delta_ms (SERVER_TICK_MS).
 */
int game_update(game_t *game, int player_count, int delta_ms);

void game_toggle_pause(game_t *game);

void game_end(game_t *game);

#endif

