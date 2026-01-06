#include "client.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  char *address = "127.0.0.1";

  if (argc < 2) {
    printf("Pouzitie: %s <port> [ip]\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  if (port <= 0 || port > 65535) {
    printf("Neplatny port\n");
    return 1;
  }

  if (argc >= 3) address = argv[2];

  client_t client;
  client_init(&client, address, port);
  client_run(&client);
  client_shutdown(&client);

  return 0;
}




