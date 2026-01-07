#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include "../common/protocol.h"

#include <unistd.h>
#include <arpa/inet.h>

typedef struct {
  int fd;
  bool connected;
} ipc_client_t;

void ipc_client_init(ipc_client_t *c);
bool ipc_client_connect(ipc_client_t *c, const char *address, int port);
bool ipc_client_send(ipc_client_t *c, const client_message_t *msg);
bool ipc_client_receive(ipc_client_t *c, server_message_t *msg);
void ipc_client_close(ipc_client_t *c);

#endif

