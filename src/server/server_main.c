#include "server.h"
#include "ipc_server.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    ipc_server_t ipc = {0};

    if (ipc_server_start(&ipc, 12345) != 0) {
        return 1;
    }

    while (1) {
        ipc_server_accept(&ipc);
        sleep(1);
    }

    return 0;
}

