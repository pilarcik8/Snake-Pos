#include "client.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Pouzitie: %s <address> <port> <player_id>\n", argv[0]);
        return 1;
    }

    const char *address = argv[1];
    int port = atoi(argv[2]);
    int player_id = atoi(argv[3]);

    client_t client;
    client_init(&client, address, port, player_id);
    client_run(&client);
    client_shutdown(&client);

    return 0;
}



