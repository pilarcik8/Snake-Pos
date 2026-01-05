#include "server.h"
#include "ipc_server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

static server_t server;
static ipc_server_t ipc;

// ukončenie pre Ctrl+C
static void handle_sigint(int sig) {
  (void)sig;
  printf("\n[SERVER] Shutting down...\n");
  server_shutdown(&server);
  _exit(0);
}

static void print_usage(const char *prog) {
  printf("Pouzitie:\n");
  printf("  %s <port>\n", prog);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  if (port <= 0 || port > 65535) {
    printf("Neplatny port\n");
    return 1;
  }

  // signaly
  signal(SIGINT, handle_sigint);
  signal(SIGPIPE, SIG_IGN);

  // init servera (nastavi game mode + time limit)
  server_init(&server);

  // start IPC (socket)
  if (ipc_server_start(&ipc, port) != 0) return 1;
  
  // spustenie vlakien
  server_run(&server, &ipc);

  // pockaj na vlakna (server_shutdown v SIGINT ich korektne ukonci)
  pthread_join(server.game_thread, NULL);
  pthread_join(server.ipc_thread, NULL);

  return 0;
}


