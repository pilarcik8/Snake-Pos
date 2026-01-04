#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include "../common/types.h"
#include "../common/config.h"

typedef struct {
    game_mode_t mode;
    int time_limit;       // len pre GAME_TIMED
    int elapsed_time;     // sekundy od štartu hry
    int running;          // 1 = beží, 0 = skončila
    int ms_accum;         // nazbierane ms pre prechod na 1 sekundu
    int empty_time;       // sekundy bez hráčov
    pthread_mutex_t lock;
} game_t;

void game_init(game_t *game);

/**
 * Aktualizuje stav hry o 1 sekundu.
 * @param player_count počet aktuálne pripojených/aktívnych hráčov (neskôr hadíkov)
 * @return 1 ak hra pokračuje, 0 ak hra skončila
 */
int game_update(game_t *game, int player_count);

void game_end(game_t *game);

#endif

