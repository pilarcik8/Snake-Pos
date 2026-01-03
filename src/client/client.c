#include "client.h"
#include "input.h"
#include "render.h"

#include <string.h>

void client_init(client_t *c,
                 const char *address,
                 int port,
                 int player_id) {
    memset(c, 0, sizeof(*c));

    c->player_id = player_id;
    c->running = true;

    ipc_client_init(&c->ipc);

    if (!ipc_client_connect(&c->ipc, address, port)) {
        c->running = false;
        return;
    }
}

void client_run(client_t *c) {
    if (!c->running) return;

    client_message_t msg;
    msg.type = MSG_CONNECT;
    msg.player_id = c->player_id;
    msg.direction = RIGHT;

    ipc_client_send(&c->ipc, &msg);

    pthread_create(&c->input_thread, NULL, input_thread_main, c);
    pthread_create(&c->render_thread, NULL, render_thread_main, c);

    pthread_join(c->input_thread, NULL);
    pthread_join(c->render_thread, NULL);
}

void client_shutdown(client_t *c) {
    if (!c->running) return;

    client_message_t msg;
    msg.type = MSG_DISCONNECT;
    msg.player_id = c->player_id;
    msg.direction = RIGHT;

    ipc_client_send(&c->ipc, &msg);
    ipc_client_close(&c->ipc);
}



