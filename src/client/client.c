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

void client_run(client_t *c) {
  if (!c->running) return;

  c->state = CLIENT_MENU;
  c->in_game_threads_started = false;

  while (c->running && c->state != CLIENT_EXIT) {
    if (c->state == CLIENT_MENU) {
      int choice = menu_read_choice();

      if (choice == 1 || choice == 2) {
        client_message_t msg;
        msg.type = MSG_CONNECT;
        msg.direction = RIGHT;
        ipc_client_send(&c->ipc, &msg);

        if (!c->in_game_threads_started) {
          c->in_game_threads_started = true;
          pthread_create(&c->input_thread, NULL, input_thread_main, c);
          pthread_create(&c->render_thread, NULL, render_thread_main, c);
        }

        c->state = CLIENT_IN_GAME;
      } 
      else if (choice == 3) {
        // zatiaľ iba prepneme stav 
        c->state = CLIENT_IN_GAME;
      } 
      else if (choice == 4) {
        c->state = CLIENT_EXIT;
        c->running = false;
      } 
      else {
        printf("Neplatna volba.\n");
      }
    }

    if (c->state == CLIENT_IN_GAME) {
      // nič nerob – input/render thready bežia
      // do menu sa dostaneme tak, že input thread nastaví c->state = CLIENT_MENU pri 'p'
      sleep(1);
    }
  }

  if (c->in_game_threads_started) {
    pthread_join(c->input_thread, NULL);
    pthread_join(c->render_thread, NULL);
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


