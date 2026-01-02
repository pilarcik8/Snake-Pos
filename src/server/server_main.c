#include "server.h"
#include "ipc_server.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static server_t server;
static ipc_server_t ipc;

static void handle_sigint(int sig) {
    (void)sig;
    printf("\n[SERVER] Shutting down...\n");
    server_shutdown(&server);
    _exit(0);
}

int main(void) {
    signal(SIGINT, handle_sigint);

    server_init(&server);

    if (ipc_server_start(&ipc, 12345) != 0) {
        return 1;
    }

    server_run(&server, &ipc);

    while (1) {
        pause();
    }
    return 0;
}

