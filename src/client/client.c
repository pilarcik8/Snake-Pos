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
  printf("1) Nova hra\n");
  printf("2) Pripojit sa k hre\n");
  printf("3) Pokracovat\n");
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
  msg.direction = RIGHT;
  ipc_client_send(&c->ipc, &msg);
}

void client_shutdown(client_t *c) {
  if (!c->running) return;

  client_message_t msg;
  msg.type = MSG_DISCONNECT;
  msg.direction = RIGHT;

  ipc_client_send(&c->ipc, &msg);
  ipc_client_close(&c->ipc);
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
  while (input != 's' && input != 't') {

    input = getchar();
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
  if (choise_mode == 1) {
    msg.cfg.world_type = WORLD_NO_OBSTACLES;
  }
  else if (choise_mode == 2) {
    msg.cfg.world_type = WORLD_WITH_OBSTACLES;
  }
  else { //TOODO: Z SUBORU
    printf("Not implemented yet\n");
    msg.cfg.world_type = WORLD_NO_OBSTACLES;

    //msg.cfg.world_type = WORLD_MAP_LOADED;
    //printf("Write a path to a file\n");
  }

  char choise_multipl = ' ';
  printf("Do you want to enable multiplayer? (y/n)\n");
  while (choise_multipl != 'y' && choise_multipl != 'n') {
    choise_multipl = getchar();
  }

  if (choise_multipl == 'y') {
    msg.cfg.allowed_multiplayer = true;
  } else {
    msg.cfg.allowed_multiplayer = false;
  }

  // TODO: POZRI CI TO TREBA ALEBO TO STACI HARD CODED + bude to robit problemy pri WORLD_MAP_LOADED
  msg.cfg.width = 50;
  msg.cfg.height = 50;

  ipc_client_send(&c->ipc, &msg);
}

void client_run(client_t *c) {
  if (!c->running) return;

  while (c->running && c->state != CLIENT_EXIT) {
    if (c->state == CLIENT_MENU) {
      int choice = menu_read_choice();

      if (choice == 1 || choice == 2) {
        if (choice == 1)  send_create_game(c); 

        send_connect(c);       

        c->state = CLIENT_IN_GAME;
        c->paused = false;

        pthread_create(&c->input_thread, NULL, input_thread_main, c);
        pthread_create(&c->render_thread, NULL, render_thread_main, c);

        pthread_join(c->input_thread, NULL);
        pthread_join(c->render_thread, NULL);
        
        // hra skoncila ale server ide -> menu
        if (c->running) c->state = CLIENT_MENU;
      }
      else if (choice == 3) {
        // zatiaľ iba “pokračovať” = nič (reálne to bude resume po pauze)
        if (!c->paused) {
          printf("Nemas pauznutu hru.\n");
        } else {
          printf("ESTE NEIMPLEMENTOVANE\n");
        }
      } 
      else if (choice == 4) {
        c->state = CLIENT_EXIT;
        c->running = false;

        //TOO: SKONTROLUJ CI TO UVOLNY SOCKET ALEBO PORT
      }
    }
  }
}




