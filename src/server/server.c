#include "server.h"
#include "game.h"
#include "../utils/time_utils.h"
#include "../common/protocol.h"
#include "../common/config.h"
#include <stdio.h>

typedef struct {
    server_t *server;
    ipc_server_t *ipc;
} server_context_t;

/* jedna inštancia hry pre tento server proces */
static game_t g_game;

/* zatiaľ len placeholder: počet hráčov */
static int get_player_count_placeholder(void) {
    // neskôr: spočítaš aktívnych hráčov/hadíkov
    return 0;
}

/* ---------------- GAME LOOP THREAD ---------------- */

static void *game_loop(void *arg) {
    server_context_t *ctx = (server_context_t *)arg;

    server_message_t state;
    state.type = MSG_STATE;
    state.game_time = 0;
    state.player_count = 0;

    while (ctx->server->running) {
        // 1 sekunda = 1 krok herného času (jednoduché a stabilné)
        sleep(1);

        int player_count = get_player_count_placeholder();

        // game_update rozhodne, či hra pokračuje
        if (!game_update(&g_game, player_count)) {
            printf("[SERVER] Game ended by game rules\n");

            // voliteľne: pošli klientom info o konci hry
            server_message_t end_msg = {0};
            end_msg.type = MSG_GAME_OVER;
            end_msg.game_time = g_game.elapsed_time;
            end_msg.player_count = player_count;
            ipc_server_send_state(ctx->ipc, &end_msg);

            ctx->server->running = 0;
            break;
        }

        state.game_time++;
        state.player_count = player_count;

        ipc_server_send_state(ctx->ipc, &state);

        // debug (kľudne zníž frekvenciu alebo vypni)
        printf("[SERVER] Tick %d\n", state.game_time);
    }

    game_end(&g_game);
    return NULL;
}

/* ---------------- IPC THREAD ---------------- */

static void *ipc_loop(void *arg) {
    server_context_t *ctx = (server_context_t *)arg;
    client_message_t msg;

    while (ctx->server->running) {
        ipc_server_accept(ctx->ipc);
        ipc_server_receive(ctx->ipc, &msg);

        // neskôr tu spracuješ MSG_INPUT / MSG_PAUSE / MSG_CONNECT / MSG_DISCONNECT
        sleep_ms(50);
    }
    return NULL;
}

/* ---------------- SERVER API ---------------- */

void server_init(server_t *server) {
    server->running = 1;

    // nastav režim hry (zatiaľ napevno)
    g_game.mode = GAME_STANDARD; // alebo GAME_TIMED
    g_game.time_limit = 60;      // použije sa len pri GAME_TIMED
    game_init(&g_game);
}

void server_run(server_t *server, ipc_server_t *ipc) {
    static server_context_t ctx;
    ctx.server = server;
    ctx.ipc = ipc;

    pthread_create(&server->game_thread, NULL, game_loop, &ctx);
    pthread_create(&server->ipc_thread, NULL, ipc_loop, &ctx);
}

void server_shutdown(server_t *server) {
    server->running = 0;
    pthread_join(server->game_thread, NULL);
    pthread_join(server->ipc_thread, NULL);
}

