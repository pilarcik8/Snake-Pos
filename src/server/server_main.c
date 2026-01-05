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
  printf("  %s <port> standard\n", prog);
  printf("  %s <port> timed <sekundy>\n", prog);
}

int main(int argc, char **argv) {
  if (argc < 3) {
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

  // vyber herneho rezimu
  game_mode_t mode;
  int time_limit = 0;

  if (strcmp(argv[2], "standard") == 0) {
    mode = GAME_STANDARD;
    time_limit = 0;
    printf("[SERVER] Game mode: STANDARD\n");

  } else if (strcmp(argv[2], "timed") == 0) {
    if (argc < 4) {
      print_usage(argv[0]);
      return 1;
    }
    int seconds = atoi(argv[3]);

    if (seconds <= 0) {
      printf("Neplatny cas\n");
      return 1;
    }

    mode = GAME_TIMED;
    time_limit = seconds;
    printf("[SERVER] Game mode: TIMED (%d sec)\n", seconds);
  } else {
    print_usage(argv[0]);
    return 1;
  }

  // init servera (nastavi game mode + time limit)
  server_init(&server, mode, time_limit);

  // start IPC (socket)
  if (ipc_server_start(&ipc, port) != 0) return 1;
  
  // spustenie vlakien
  server_run(&server, &ipc);

  // pockaj na vlakna (server_shutdown v SIGINT ich korektne ukonci)
  pthread_join(server.game_thread, NULL);
  pthread_join(server.ipc_thread, NULL);

  return 0;
}


