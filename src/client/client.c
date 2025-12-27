#include "client.h"
#include <stdio.h>

void client_start(client_t *client) {
    client->running = 1;
    client->player_id = 0;
    printf("Client started\n");
}

void client_stop(client_t *client) {
    client->running = 0;
    printf("Client stopped\n");
}

