#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include "game.h"

typedef struct {
    int running;
    pthread_t game_thread;
    pthread_t ipc_thread;
} server_t;

void server_init(server_t *server);
void server_run(server_t *server);
void server_shutdown(server_t *server);

#endif

