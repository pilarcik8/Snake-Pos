#ifndef CLIENT_H
#define CLIENT_H

#include "ipc_client.h"

#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

typedef enum {
  CLIENT_MENU,
  CLIENT_IN_GAME,
  CLIENT_EXIT
} client_state_t;

typedef struct {
  ipc_client_t ipc;

  int player_id;
  bool running;

  client_state_t state;
  bool paused;

  pthread_t input_thread;
  pthread_t render_thread;
} client_t;

void client_init(client_t *c);
void client_run(client_t *c);
void client_shutdown(client_t *c);

bool client_connect_to(client_t *c, const char *address, int port);

#endif






