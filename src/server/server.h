#ifndef SERVER_H
#define SERVER_H

#include "ipc_server.h"

#include <pthread.h>

typedef struct {
  bool running;
  pthread_t game_thread;
  pthread_t ipc_thread;
  ipc_server_t *ipc;
  bool game_running;
} server_t;


void server_init(server_t *server);
void server_run(server_t *server, ipc_server_t *ipc);
void server_shutdown(server_t *server);

#endif

