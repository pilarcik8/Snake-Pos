#define _POSIX_C_SOURCE 200809L

#include "client.h"
#include "input.h"
#include "render.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

static int read_int_safe(int *out) {
  int rc = scanf("%d", out);
  if (rc == 1) return 1;

  // vyhoď zbytok riadku
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF) {}
  return 0;
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

  while (!read_int_safe(&c) || c < 1 || c > 4) {
    printf("Invalid value, try again:\n> ");
  }
  return c;
}

static int barrier_choice(void) {
  int c = 0;

  printf("Choose game type:\n");
  printf("1) Game word without barries\n");
  printf("2) Game word with barries - random\n");
  printf("3) Game word inputed from a file\n");
  printf("> ");

  while (!read_int_safe(&c) ||  c < 1 || c > 3) {
    printf("Invalid value, try again:\n> ");
  }
  return c;
}

static int world_type_choice(void) {
  int c = 0;

  printf("Choose game mode:\n");
  printf("1) Standard\n");
  printf("2) Timed\n");
  printf("> ");

  while (!read_int_safe(&c) ||  c < 1 || c > 3) {
    printf("Invalid value, try again:\n> ");
  }
  return c;
}

static bool multiplayer_choice(void) {
  int c = 0;
  printf("Do you want to enable multiplayer?\n");
  printf("1) Yes\n");
  printf("2) No\n");
  printf("> ");

  while (!read_int_safe(&c) ||  c < 1 || c > 2) {
    printf("Invalid value, try again:\n> ");
  }

  if (c == 2) {
    c = 0;
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

  msg.cfg.mode = world_type_choice();
  int time_limit_sec = 0;
  if (msg.cfg.mode == GAME_TIMED) {
    printf("Write time limit in seconds:\n> ");

    while (time_limit_sec <= 0) {
      scanf("%d", &time_limit_sec);
    }
  }
  msg.cfg.time_limit_sec = time_limit_sec;

  msg.cfg.world_type = barrier_choice();

  msg.cfg.allowed_multiplayer = multiplayer_choice();

  int width = 0;
  int height = 0;

  if (msg.cfg.world_type == WORLD_WITH_OBSTACLES || msg.cfg.world_type == WORLD_NO_OBSTACLES) {
    printf("Write width of the map (min 10):\n> ");

    while (!read_int_safe(&width) || width < 10) {
      printf("Invalid value, try again:\n> ");
    }

    printf("Write height of the map (min 10):\n> ");

    while (!read_int_safe(&height) || height < 10) {
      printf("Invalid value, try again:\n> ");
    }
  }
  msg.cfg.width = width;
  msg.cfg.height = height;
  

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
      // ak hrac ide pauza -> nova hra
      if (c->paused && c->ipc.connected) {
        client_message_t m; memset(&m,0,sizeof(m));
        m.type = MSG_DISCONNECT;
        ipc_client_send(&c->ipc, &m);
        ipc_client_close(&c->ipc);
        c->paused = false;
      }

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
      // ak hrac ide pauza -> pripojit sa do inej hry
      if (c->paused && c->ipc.connected) {
        client_message_t m; memset(&m,0,sizeof(m));
        m.type = MSG_DISCONNECT;
        ipc_client_send(&c->ipc, &m);
        ipc_client_close(&c->ipc);
        c->paused = false;
      }

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
      if (!c->paused || !c->ipc.connected) {
        printf("Nemas pauznutu hru.\n");
        continue;
      }

      // návrat do hry
      c->state = CLIENT_IN_GAME;
      c->paused = false;

      // toto na serveri spustí 3s resume (podľa tvojej logiky v MSG_CONNECT)
      send_connect(c);

      start_game_threads(c);  // znovu input + render

      if (c->running) c->state = CLIENT_MENU;
    }
    else if (choice == 4) {
      c->state = CLIENT_EXIT;
      client_shutdown(c);        // odpojí (ak je pripojený = pauza) + zavrie socket
      c->running = false;
    }
  }
}


