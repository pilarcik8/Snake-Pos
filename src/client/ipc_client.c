#include "ipc_client.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int ipc_client_connect(const char *address, int port) {
    struct sockaddr_in server_addr;
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &server_addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect");
        return -1;
    }

    printf("Connected to server %s:%d\n", address, port);
    return fd;
}

void ipc_client_send(int fd, client_message_t *msg) {
    send(fd, msg, sizeof(*msg), 0);
}

void ipc_client_receive(int fd, server_message_t *msg) {
    recv(fd, msg, sizeof(*msg), 0);
}

