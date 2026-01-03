#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <pthread.h>

#include "ipc_client.h"
#include "../common/protocol.h"

// Stav klienta.
typedef struct {
    ipc_client_t ipc;

    int player_id;
    bool running;

    pthread_t input_thread;
    pthread_t render_thread;
} client_t;

// Inicializuje klienta a pripoji ho na server.
void client_init(client_t *c,
                 const char *address,
                 int port,
                 int player_id);

void client_run(client_t *c);
void client_shutdown(client_t *c);

#endif



