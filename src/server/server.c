#include "server.h"
#include <stdio.h>
#include <unistd.h>

void server_init(server_t *server) {
    server->running = 1;
    printf("Server initialized\n");
}

void server_run(server_t *server) {
    printf("Server running\n");
    while (server->running) {
        sleep(1);
    }
}

void server_shutdown(server_t *server) {
    printf("Server shutting down\n");
}

