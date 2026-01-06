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

static void handle_sigint(int sig) {
  (void)sig;
  printf("\n[SERVER] Shutting down...\n");
  server_shutdown(&server);
  _exit(0);
}

static void print_usage(const char *prog) {
  printf("Pouzitie:\n");
  printf("  %s <port> [port_fd]\n", prog);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  if (port < 0 || port > 65535) {
    printf("Neplatny port\n");
    return 1;
  }

  int port_fd = -1;
  if (argc >= 3) {
    port_fd = atoi(argv[2]);
    if (port_fd < 0) port_fd = -1;
  }

  signal(SIGINT, handle_sigint);
  signal(SIGPIPE, SIG_IGN);

  server_init(&server);

  if (ipc_server_start(&ipc, port) != 0) return 1;

  // ak klient poslal FD, zapis tam realny port a zavri
  if (port_fd != -1) {
    dprintf(port_fd, "%d\n", ipc.listen_port);
    close(port_fd);
  }

  server_run(&server, &ipc);

  pthread_join(server.game_thread, NULL);
  pthread_join(server.ipc_thread, NULL);

  printf("\n[SERVER] Treads down...\n");
  return 0;
}


