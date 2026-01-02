#include "server.h"
#include "game.h"
#include "../common/protocol.h"
#include "../common/config.h"
#include <stdio.h>
#include <unistd.h>  

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

    // stav, ktorý server posiela klientom
    server_message_t state = {0};
    state.type = MSG_STATE;
    state.game_time = 0;
    state.player_count = 0;

    while (ctx->server->running) {
        // 1 sekunda = 1 herný krok (zat. jednoduché)
        sleep(1);

        // zatiaľ placeholder (neskôr spočítaš aktívnych hráčov / hadíkov)
        int player_count = get_player_count_placeholder();

        // aktualizuj herný čas a skontroluj koniec hry
        if (!game_update(&g_game, player_count)) {
            printf("[SERVER] Game ended by game rules\n");

            // pošli klientom, že hra skončila (voliteľné, ale dobré)
            server_message_t end_msg = {0};
            end_msg.type = MSG_GAME_OVER;
            end_msg.game_time = g_game.elapsed_time;
            end_msg.player_count = player_count;
            ipc_server_send_state(ctx->ipc, &end_msg);

            // ukonči server
            ctx->server->running = 0;
            break;
        }

        // posuň čas v správe a odošli stav
        state.game_time++;
        state.player_count = player_count;

        ipc_server_send_state(ctx->ipc, &state);

        // debug výpis (môžeš vypnúť alebo zriediť)
        printf("[SERVER] Tick %d\n", state.game_time);
    }

    // upratanie game modulu
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
        sleep(1);
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

