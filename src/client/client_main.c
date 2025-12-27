#include "ipc_client.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    int fd = ipc_client_connect("127.0.0.1", 12345);
    if (fd < 0) return 1;

    client_message_t msg;
    msg.type = MSG_CONNECT;
    msg.player_id = 1;
    msg.direction = UP;

    ipc_client_send(fd, &msg);

    while (1) {
        sleep(1);
    }

    return 0;
}

