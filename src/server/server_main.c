#define _POSIX_C_SOURCE 200809L

#include "server.h"
#include "ipc_server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

// globály
static server_t server;
static ipc_server_t ipc;

// Ctrl+C
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
    port_fd = atoi(argv[2]);   // fd kam mame zapisat port
  }

  signal(SIGINT, handle_sigint);
  signal(SIGPIPE, SIG_IGN);

  server_init(&server);

  // port == 0 → OS vyberie voľný port
  if (ipc_server_start(&ipc, port) != 0) {
    return 1;
  }

  /*
  * AK SME BOLI SPAWNUTÍ KLIENTOM:
  * zapíšeme port cez write() do pipe
   */
  if (port == 0 && port_fd >= 0) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", ipc.listen_port);
    if (len > 0) {
      write(port_fd, buf, (size_t)len);
    }
    close(port_fd);
  }

  server_run(&server, &ipc);

  pthread_join(server.game_thread, NULL);
  pthread_join(server.ipc_thread, NULL);

  printf("[SERVER] Threads down.\n");
  return 0;
}



