#include "client.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Pouzitie: %s <address> <port>\n", argv[0]);
        return 1;
    }

    const char *address = argv[1];
    int port = atoi(argv[2]);

    client_t client;
    client_init(&client, address, port);
    client_run(&client);
    client_shutdown(&client);

    return 0;
}




