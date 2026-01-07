#include "ipc_server.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

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
  addr.sin_port = htons((uint16_t)port);

  if (bind(ipc->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(ipc->server_fd);
    return -1;
  }

  
  socklen_t alen = sizeof(addr);
  getsockname(ipc->server_fd, (struct sockaddr *)&addr, &alen);
  port = ntohs(addr.sin_port);
  ipc->listen_port = port;

  if (listen(ipc->server_fd, MAX_PLAYERS) < 0) {
    perror("listen");
    close(ipc->server_fd);
    return -1;
  }

  int flags = fcntl(ipc->server_fd, F_GETFL, 0);
  if (flags >= 0) {
    fcntl(ipc->server_fd, F_SETFL, flags | O_NONBLOCK);
  }

  pthread_mutex_init(&ipc->lock, NULL);

  // Inicializacia client slotov
  for (int i = 0; i < MAX_PLAYERS; i++) {
      ipc->client_fds[i] = 0;
      ipc->client_player_id[i] = -1;
  }

  ipc->next_player_id = 1;

  printf("Server listening on port %d\n", port);
  return 0;
}

int ipc_server_accept(ipc_server_t *ipc) {
  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);

  int client_fd = accept(ipc->server_fd, (struct sockaddr *)&client_addr, &len);
  if (client_fd < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return -1;
    return -1;
  }

  pthread_mutex_lock(&ipc->lock);

  int slot = -1;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (ipc->client_fds[i] == 0) { slot = i; break; }
  }

  if (slot < 0) {
    pthread_mutex_unlock(&ipc->lock);
    close(client_fd);
    return -1;
  }

  ipc->client_fds[slot] = client_fd;
  ipc->client_player_id[slot] = ipc->next_player_id++;
  printf("Client connected (slot %d, id %d)\n", slot, ipc->client_player_id[slot]);

  pthread_mutex_unlock(&ipc->lock);
  return slot;
}

int ipc_server_receive(ipc_server_t *ipc, client_message_t *msg) {
  pthread_mutex_lock(&ipc->lock);

  for (int i = 0; i < MAX_PLAYERS; i++) {
    int fd = ipc->client_fds[i];
    if (fd <= 0) continue;

    ssize_t r = recv(fd, msg, sizeof(*msg), MSG_DONTWAIT);

    if (r == 0) {
      close(fd);
      ipc->client_fds[i] = 0;

      msg->type = MSG_DISCONNECT;
      pthread_mutex_unlock(&ipc->lock);
      return i; // slot
    }

    if (r == (ssize_t)sizeof(*msg)) {
      pthread_mutex_unlock(&ipc->lock);
      return i; // slot
    }
  }

  pthread_mutex_unlock(&ipc->lock);
  return -1; // nič neprišlo
}

void ipc_server_send_state(ipc_server_t *ipc, server_message_t *state) {
  pthread_mutex_lock(&ipc->lock);

  for (int i = 0; i < MAX_PLAYERS; i++) {
    int fd = ipc->client_fds[i];
    if (fd <= 0) continue;

    ssize_t w = send(fd, state, sizeof(*state), 0);
    if (w != (ssize_t)sizeof(*state)) {
      close(fd);
      ipc->client_fds[i] = 0;
      ipc->client_player_id[i] = -1;
    }
  }
  pthread_mutex_unlock(&ipc->lock);
}

void ipc_server_kick(ipc_server_t *ipc, int slot) {
  pthread_mutex_lock(&ipc->lock);

  int fd = ipc->client_fds[slot];
  if (fd > 0) close(fd);

  ipc->client_fds[slot] = 0;
  ipc->client_player_id[slot] = -1;

  pthread_mutex_unlock(&ipc->lock);
}



