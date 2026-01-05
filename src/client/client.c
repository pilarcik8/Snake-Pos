#include "client.h"
#include "input.h"
#include "render.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

void client_init(client_t *c, const char *address, int port) {
  memset(c, 0, sizeof(*c));
  c->running = true;

  ipc_client_init(&c->ipc);

  if (!ipc_client_connect(&c->ipc, address, port)) {
    c->running = false;
    return;
  }
}

static int menu_read_choice(void) {
  int c = 0;
  printf("\n=== MENU ===\n");
  printf("1) Nova hra (zatim iba connect)\n");
  printf("2) Pripojit sa k hre\n");
  printf("3) Pokracovat (ak si pauzol)\n");
  printf("4) Koniec\n");
  printf("> ");
  fflush(stdout);

  if (scanf("%d", &c) != 1) {
    // vycisti zly vstup
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    return 0;
  }
  return c;
}

static void send_connect(client_t *c) {
  client_message_t msg;
  msg.type = MSG_CONNECT;
  msg.direction = RIGHT; // ignoruj na serveri pri CONNECT
  ipc_client_send(&c->ipc, &msg);
}

void client_run(client_t *c) {
  if (!c->running) return;

  while (c->running && c->state != CLIENT_EXIT) {
    if (c->state == CLIENT_MENU) {
      int choice = menu_read_choice();

      if (choice == 1 || choice == 2) {
        send_connect(c);

        c->state = CLIENT_IN_GAME;
        c->paused = false;

        pthread_create(&c->input_thread, NULL, input_thread_main, c);
        pthread_create(&c->render_thread, NULL, render_thread_main, c);

        pthread_join(c->input_thread, NULL);
        pthread_join(c->render_thread, NULL);

        // keď thready skončia, vráť sa do menu (ak klient ešte beží)
        if (c->running) c->state = CLIENT_MENU;
      } 
      else if (choice == 3) {
        // zatiaľ iba “pokračovať” = nič (reálne to bude resume po pauze)
        printf("Nemas pauznutu hru.\n");
      } 
      else if (choice == 4) {
        c->state = CLIENT_EXIT;
        c->running = false;
      }
    }
  }
}

void client_shutdown(client_t *c) {
  if (!c->running) return;

  client_message_t msg;
  msg.type = MSG_DISCONNECT;
  msg.direction = RIGHT;

  ipc_client_send(&c->ipc, &msg);
  ipc_client_close(&c->ipc);
}


