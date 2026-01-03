#include "render.h"
#include "client.h"

#include <stdio.h>
#include <string.h>

static void clear_screen(void) {
    printf("\033[2J\033[H");
}

static void draw_state(const server_message_t *st) {
    clear_screen();
    printf("Cas: %d | Hracov: %d\n", st->game_time, st->player_count);
    printf("WASD pohyb | p pauza | q odchod\n\n");

    for (int s = 0; s < MAX_PLAYERS; s++) {
        if (st->snakes[s].player_id < 0) continue;
        printf("Hrac %d: %d\n", st->snakes[s].player_id, st->snakes[s].score);
    }

    printf("\nOvocie: %d\n", st->fruit_count);
}

void *render_thread_main(void *arg) {
    client_t *c = (client_t *)arg;

    while (c->running) {
        server_message_t st;
        memset(&st, 0, sizeof(st));

        if (!ipc_client_receive(&c->ipc, &st)) {
            c->running = false;
            break;
        }

        if (st.type == MSG_GAME_OVER) {
            clear_screen();
            printf("Koniec hry. Cas: %d\n", st.game_time);
            c->running = false;
            break;
        }

        if (st.type == MSG_STATE) {
            draw_state(&st);
        }
    }

    return NULL;
}


