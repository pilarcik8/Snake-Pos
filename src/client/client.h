#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <pthread.h>

#include "ipc_client.h"

typedef enum {
  CLIENT_MENU,
  CLIENT_IN_GAME,
  CLIENT_EXIT
} client_state_t;

typedef struct {
  ipc_client_t ipc;

  int player_id;
  bool running;   //true - klient bezi, false - klient sa ma ukoncit

  client_state_t state;
  bool paused;

  bool in_game_threads_started;
  pthread_t input_thread;
  pthread_t render_thread;
} client_t;

// Inicializuje klienta a pripoji ho na server.
void client_init(client_t *c, const char *address, int port);

void client_run(client_t *c);
void client_shutdown(client_t *c);

#endif



