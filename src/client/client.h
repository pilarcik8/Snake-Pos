#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include "../common/types.h"

typedef struct {
    int running;
    int player_id;
    pthread_t input_thread;
    pthread_t render_thread;
} client_t;

void client_start(client_t *client);
void client_stop(client_t *client);

#endif

