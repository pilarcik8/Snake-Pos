#define _POSIX_C_SOURCE 200809L
#include "client.h"
#include "input.h"
#include "render.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <errno.h>

void client_init(client_t *c) {
  memset(c, 0, sizeof(*c));
  c->running = true;
  c->state = CLIENT_MENU;
  ipc_client_init(&c->ipc);
}

bool client_connect_to(client_t *c, const char *address, int port) {
  if (c->ipc.connected) ipc_client_close(&c->ipc);
  return ipc_client_connect(&c->ipc, address, port);
}

static int menu_read_choice(void) {
  int c = 0;
  printf("\n=== MENU ===\n");
  printf("1) Nova hra\n");
  printf("2) Pripojit sa k hre\n");
  printf("3) Pokracovat\n");
  printf("4) Koniec\n");
  printf("> ");
  fflush(stdout);

  if (scanf("%d", &c) != 1) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    return 0;
  }
  return c;
}

static void send_connect(client_t *c) {
  client_message_t msg;
  memset(&msg, 0, sizeof(msg));
  msg.type = MSG_CONNECT;
  msg.direction = RIGHT;
  ipc_client_send(&c->ipc, &msg);
}

static void send_create_game(client_t *c) {
  client_message_t msg;
  memset(&msg, 0, sizeof(msg));
  msg.type = MSG_CREATE_GAME;

  char input = ' ';
  int time_limit_sec = 0;

  printf("Choose game mode:\n");
  printf("s) Standart\n");
  printf("t) Timed\n");

  // zjedz whitespace po scanf v menu
  do { input = (char)getchar(); } while (input == '\n' || input == '\r' || input == ' ' || input == '\t');

  while (input != 's' && input != 't') {
    input = (char)getchar();
  }

  msg.cfg.mode = GAME_STANDARD;
  if (input == 't') {
    msg.cfg.mode = GAME_TIMED;
    printf("Write time limit in seconds:\n");
    while (time_limit_sec <= 0) {
      scanf("%d", &time_limit_sec);
    }
  }

  msg.cfg.time_limit_sec = time_limit_sec;

  int choise_mode = 0;
  printf("Choose game type:\n");
  printf("1) Game word without barries\n");
  printf("2) Game word with barries\n");
  printf("3) Game word inputed from a file\n");

  while (choise_mode < 1 || choise_mode > 3) {
    scanf("%d", &choise_mode);
  }

  if (choise_mode == 1) msg.cfg.world_type = WORLD_NO_OBSTACLES;
  else if (choise_mode == 2) msg.cfg.world_type = WORLD_WITH_OBSTACLES;
  else {
    printf("Not implemented yet\n");
    msg.cfg.world_type = WORLD_NO_OBSTACLES;
  }

  char choise_multipl = ' ';
  printf("Do you want to enable multiplayer? (y/n)\n");
  do { choise_multipl = (char)getchar(); } while (choise_multipl == '\n' || choise_multipl == '\r' || choise_multipl == ' ' || choise_multipl == '\t');

  while (choise_multipl != 'y' && choise_multipl != 'n') {
    choise_multipl = (char)getchar();
  }

  msg.cfg.allowed_multiplayer = (choise_multipl == 'y');

  msg.cfg.width = 50;
  msg.cfg.height = 30;

  ipc_client_send(&c->ipc, &msg);
}

static int spawn_server_and_get_port(void) {
  int pfd[2];
  if (pipe(pfd) < 0) { perror("pipe"); return -1; }

  pid_t pid = fork();
  if (pid < 0) { perror("fork"); close(pfd[0]); close(pfd[1]); return -1; }

  if (pid == 0) {
    close(pfd[0]);

    char fdstr[32];
    snprintf(fdstr, sizeof(fdstr), "%d", pfd[1]);

    char *argv[] = { "./server", "0", fdstr, NULL };
    execv(argv[0], argv);

    perror("execv");
    _exit(127);
  }

  close(pfd[1]);

  char buf[64];
  ssize_t n = read(pfd[0], buf, sizeof(buf) - 1);
  close(pfd[0]);

  if (n <= 0) return -1;
  buf[n] = '\0';

  int port = atoi(buf);
  if (port <= 0 || port > 65535) return -1;
  return port;
}

static bool prompt_join_info(char *ip_out, size_t ip_cap, int *port_out) {
  if (!ip_out || ip_cap == 0 || !port_out) return false;

  printf("Zadaj IP (enter = 127.0.0.1): ");
  fflush(stdout);

  // zjedz newline
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF) {}

  if (!fgets(ip_out, (int)ip_cap, stdin)) return false;

  // odstrihni \n
  size_t len = strlen(ip_out);
  while (len > 0 && (ip_out[len - 1] == '\n' || ip_out[len - 1] == '\r')) {
    ip_out[--len] = '\0';
  }

  if (len == 0) {
    strncpy(ip_out, "127.0.0.1", ip_cap);
    ip_out[ip_cap - 1] = '\0';
  }

  printf("Zadaj PORT: ");
  fflush(stdout);

  int port = 0;
  if (scanf("%d", &port) != 1) return false;
  if (port <= 0 || port > 65535) return false;

  *port_out = port;
  return true;
}

void client_shutdown(client_t *c) {
  if (!c->running) return;

  if (c->ipc.connected) {
    client_message_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_DISCONNECT;
    msg.direction = RIGHT;
    ipc_client_send(&c->ipc, &msg);
    ipc_client_close(&c->ipc);
  }
}

static void start_game_threads(client_t *c) {
  pthread_create(&c->input_thread, NULL, input_thread_main, c);
  pthread_create(&c->render_thread, NULL, render_thread_main, c);

  pthread_join(c->input_thread, NULL);
  pthread_join(c->render_thread, NULL);
}

void client_run(client_t *c) {
  if (!c->running) return;

  while (c->running && c->state != CLIENT_EXIT) {
    if (c->state != CLIENT_MENU) continue;

    int choice = menu_read_choice();

    if (choice == 1) {
      int port = spawn_server_and_get_port();
      if (port < 0) {
        printf("Server sa nepodarilo spustit.\n");
        continue;
      }

      if (!client_connect_to(c, "127.0.0.1", port)) {
         c->state = CLIENT_IN_GAME;
        c->paused = false;       
        printf("Nepodarilo sa pripojit na novy server (port %d).\n", port);
        continue;
      }

      c->state = CLIENT_IN_GAME;
      c->paused = false;

      send_create_game(c);
      send_connect(c);

      start_game_threads(c);

      if (c->running) c->state = CLIENT_MENU;
    }
    else if (choice == 2) {
      char ip[64];
      int port = 0;

      if (!prompt_join_info(ip, sizeof(ip), &port)) {
        printf("Zle zadane IP/PORT.\n");
        continue;
      }

      if (!client_connect_to(c, ip, port)) {
        printf("Nepodarilo sa pripojit na %s:%d\n", ip, port);
        continue;
      }

      c->state = CLIENT_IN_GAME;
      c->paused = false;

      send_connect(c);

      start_game_threads(c);

      if (c->running) c->state = CLIENT_MENU;
    }
    else if (choice == 3) {
      printf("Pokračovanie zatiaľ nie je implementované.\n");
    }
    else if (choice == 4) {
      c->state = CLIENT_EXIT;
      c->running = false;
    }
  }
}


