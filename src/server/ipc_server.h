#ifndef IPC_SERVER_H
#define IPC_SERVER_H

#include "../common/protocol.h"

#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
  int server_fd;
  int client_fds[MAX_PLAYERS];

  int client_player_id[MAX_PLAYERS]; // ID priradene slotu
  int next_player_id;                // dalsie ID pre nove pripojenie

  pthread_mutex_t lock;

  int listen_port;
} ipc_server_t;

int ipc_server_start(ipc_server_t *ipc, int port);
int ipc_server_accept(ipc_server_t *ipc);

int ipc_server_receive(ipc_server_t *ipc, client_message_t *msg);
void ipc_server_send_state(ipc_server_t *ipc, server_message_t *state);

#endif
