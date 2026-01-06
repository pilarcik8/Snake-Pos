#include "client.h"
#include "input.h"
#include "render.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

void client_init(client_t *c) {
  memset(c, 0, sizeof(*c));
  c->running = true;
  ipc_client_init(&c->ipc);
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
//
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
  msg.cfg.height = 30;

  ipc_client_send(&c->ipc, &msg);
}

  static int spawn_server_and_get_port(void) {
    int pfd[2];
    if (pipe(pfd) < 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(pfd[0]);
        close(pfd[1]);
        return -1;
    }

    if (pid == 0) {
        // child: redirect stdout -> pipe
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        dup2(pfd[1], STDERR_FILENO); // nech vidíš aj chyby
        close(pfd[1]);

        // server s portom 0 => OS pridelí voľný port
        char *argv[] = { "./server", "0", NULL };
        execv(argv[0], argv);

        perror("execv");
        _exit(127);
    }

    // parent: čítaj výstup servera a vyparsuj port
    close(pfd[1]);

    char buf[256];
    int port = -1;

    // jednoduché čítanie po riadkoch (stačí na "Server listening on port %d\n")
    FILE *fp = fdopen(pfd[0], "r");
    if (!fp) {
        perror("fdopen");
        close(pfd[0]);
        return -1;
    }

    // Čítaj pár riadkov, kým nenájdeš port
    for (int i = 0; i < 20; i++) {
        if (!fgets(buf, sizeof(buf), fp)) break;

        int p;
        if (sscanf(buf, "Server listening on port %d", &p) == 1) {
            port = p;
            break;
        }
    }

    fclose(fp);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Nepodarilo sa zistit port servera.\n");
        return -1;
    }

    // DÔLEŽITÉ: parent NESMIE čakať na server (P5) => žiadne waitpid tu
    return port;
}
 
void client_run(client_t *c) {
  if (!c->running) return;

  while (c->running && c->state != CLIENT_EXIT) {
    if (c->state == CLIENT_MENU) {
      int choice = menu_read_choice();

      if (choice == 1) {
        int port = spawn_server_and_get_port();
        if (port < 0) { /* vypis a return do menu */ }

        if (!client_connect_to(c, "127.0.0.1", port)) { /* fail */ }

        // teraz posli create_game, connect, spusti thready...
        c->state = CLIENT_IN_GAME;
        c->paused = false;
        send_create_game(c);
        send_connect(c);
      }
      else if (choice == 2) {
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
 
bool client_connect_to(client_t *c, const char *address, int port) {
  // ak už si bol pripojený, zavri staré spojenie
  if (c->ipc.connected) ipc_client_close(&c->ipc);

  return ipc_client_connect(&c->ipc, address, port);
}

