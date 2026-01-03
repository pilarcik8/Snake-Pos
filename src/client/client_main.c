#include "client.h"

int main(int argc, char **argv) {
    const char *address = "127.0.0.1";
    if (argc >= 2) {
        address = argv[1];
    }

    client_t client;
    client_init(&client, address);
    client_run(&client);
    client_shutdown(&client);

    return 0;
}




