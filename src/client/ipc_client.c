#include "ipc_client.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

void ipc_client_init(ipc_client_t *c) {
  c->fd = -1;
  c->connected = false;
}

bool ipc_client_connect(ipc_client_t *c, const char *address, int port) {
  struct sockaddr_in server_addr;

  int fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tpc, 0
  if (fd < 0) {
    perror("socket");
    return false;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET; //ipv4
  server_addr.sin_port = htons((uint16_t)port); 
  
  // ip -> binarka
  if (inet_pton(AF_INET, address, &server_addr.sin_addr) != 1) {
    perror("inet_pton");
    close(fd);
    return false;
  }

  if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("connect");
    close(fd);
    return false;
  }

  c->fd = fd;
  c->connected = true;
  return true;
}

static bool send_all(int fd, const void *buf, size_t len) {
  const char *p = (const char *)buf;
  size_t sent = 0;

  while (sent < len) {
    ssize_t n = send(fd, p + sent, len - sent, 0);
    if (n <= 0) return false;
    sent += (size_t)n;
  }
  return true;
}

static bool recv_all(int fd, void *buf, size_t len) {
  char *p = (char *)buf;
  size_t recvd = 0;

  while (recvd < len) {
    ssize_t n = recv(fd, p + recvd, len - recvd, 0);
    if (n <= 0) return false;
    recvd += (size_t)n;
  }
  return true;
}

bool ipc_client_send(ipc_client_t *c, const client_message_t *msg) {
  if (!c->connected) return false;
  return send_all(c->fd, msg, sizeof(*msg));
}

bool ipc_client_receive(ipc_client_t *c, server_message_t *msg) {
  if (!c->connected) return false;
  return recv_all(c->fd, msg, sizeof(*msg));
}

void ipc_client_close(ipc_client_t *c) {
  if (c->fd >= 0) {
    close(c->fd);
  }
  c->fd = -1;
  c->connected = false;
}

