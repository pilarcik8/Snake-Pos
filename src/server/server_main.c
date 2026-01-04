#include "server.h"
#include "ipc_server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static server_t server;
static ipc_server_t ipc;

static void handle_sigint(int sig) {
    (void)sig;
    printf("\n[SERVER] Shutting down...\n");
    server_shutdown(&server);
    _exit(0);
}

int main(int argc, char **argv) {
    int port = 0;
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port < 0 || port > 65535) {
            printf("Pouzitie: %s [port]\n", argv[0]);
            return 1;
        }
    }

    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, SIG_IGN);

    server_init(&server);

    if (ipc_server_start(&ipc, port) != 0) {
        return 1;
    }

    server_run(&server, &ipc);

    pthread_join(server.game_thread, NULL);
    pthread_join(server.ipc_thread, NULL);
    return 0;
}



