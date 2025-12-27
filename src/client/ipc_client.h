#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include "../common/protocol.h"

int ipc_client_connect(const char *address, int port);
void ipc_client_send(int fd, client_message_t *msg);
void ipc_client_receive(int fd, server_message_t *msg);

#endif

