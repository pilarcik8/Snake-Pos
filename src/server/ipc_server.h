#ifndef IPC_SERVER_H
#define IPC_SERVER_H

#include <pthread.h>
#include "../common/protocol.h"

typedef struct {
    int server_fd;
    int client_fds[MAX_PLAYERS];
    pthread_mutex_t lock;
} ipc_server_t;

int ipc_server_start(ipc_server_t *ipc, int port);
void ipc_server_accept(ipc_server_t *ipc);
void ipc_server_receive(ipc_server_t *ipc, client_message_t *msg);
void ipc_server_send_state(ipc_server_t *ipc, server_message_t *state);

#endif

