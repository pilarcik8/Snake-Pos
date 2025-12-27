#include "ipc_server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int ipc_server_start(ipc_server_t *ipc, int port) {
    struct sockaddr_in addr;

    ipc->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ipc->server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(ipc->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(ipc->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    if (listen(ipc->server_fd, MAX_PLAYERS) < 0) {
        perror("listen");
        return -1;
    }

    pthread_mutex_init(&ipc->lock, NULL);

    printf("Server listening on port %d\n", port);
    return 0;
}

void ipc_server_accept(ipc_server_t *ipc) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int client_fd = accept(ipc->server_fd,
                            (struct sockaddr *)&client_addr, &len);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    pthread_mutex_lock(&ipc->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ipc->client_fds[i] == 0) {
            ipc->client_fds[i] = client_fd;
            printf("Client connected (slot %d)\n", i);
            break;
        }
    }
    pthread_mutex_unlock(&ipc->lock);
}

void ipc_server_receive(ipc_server_t *ipc, client_message_t *msg) {
    pthread_mutex_lock(&ipc->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        int fd = ipc->client_fds[i];
        if (fd > 0) {
            ssize_t r = recv(fd, msg, sizeof(*msg), MSG_DONTWAIT);
            if (r > 0) {
                printf("Received message from player %d\n", msg->player_id);
            }
        }
    }
    pthread_mutex_unlock(&ipc->lock);
}

void ipc_server_send_state(ipc_server_t *ipc, server_message_t *state) {
    pthread_mutex_lock(&ipc->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        int fd = ipc->client_fds[i];
        if (fd > 0) {
            send(fd, state, sizeof(*state), 0);
        }
    }
    pthread_mutex_unlock(&ipc->lock);
}


