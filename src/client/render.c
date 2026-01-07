#include "render.h"
#include "client.h"

#include <stdio.h>
#include <string.h>

static void clear_screen(void) {
  printf("\033[2J\033[H");
}

static void draw_state(const server_message_t *st) {
  clear_screen();

  printf("Cas: %d | Hracov: %d\n", st->game_time, st->player_count);
  printf("WASD pohyb | p pauza | q odchod\n\n");

  const int W = st->width;
  const int H = st->height;

  // lokálny buffer na ASCII mapu
  char grid[H][W];
    
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      if (st->cells[y][x] == CELL_WALL) {
        grid[y][x] = '#';
      } 
      else if (st->cells[y][x] == CELL_FRUIT) {
        grid[y][x] = '*';
      }
      else if (st->cells[y][x] == CELL_EMPTY) {
        grid[y][x] = '.';
      }
    }
  }

  for (int s = 0; s < MAX_PLAYERS; s++) {
    if (!st->snakes[s].alive) continue;
    if (st->snakes[s].length <= 0) continue;

    // telo (index 1..)
    for (int i = 1; i < st->snakes[s].length; i++) {
      int x = st->snakes[s].body[i].x;
      int y = st->snakes[s].body[i].y;
      if (x >= 0 && x < W && y >= 0 && y < H) {
        grid[y][x] = 'o';
        }
      }
    }

    // hlavy (index 0)
  for (int s = 0; s < MAX_PLAYERS; s++) {
    if (!st->snakes[s].alive) continue;
    if (st->snakes[s].length <= 0) continue;

    int x = st->snakes[s].body[0].x;
    int y = st->snakes[s].body[0].y;
    if (x >= 0 && x < W && y >= 0 && y < H) {
      grid[y][x] = '@';
    }
  }
 
  // vypíš grid
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      putchar(grid[y][x]);
    }
    putchar('\n');
  }

  // skóre
  printf("\nSkore:\n");
  for (int s = 0; s < MAX_PLAYERS; s++) {
    if (st->snakes[s].player_id < 0) continue;
    printf("Hrac %d: %d\n", st->snakes[s].player_id, st->snakes[s].score);
  } 
}

void *render_thread_main(void *arg) {
  client_t *c = (client_t *)arg;

  while (c->running) {
    server_message_t st;
    memset(&st, 0, sizeof(st));

    if (!ipc_client_receive(&c->ipc, &st)) {
        // spojenie padlo alebo sme sa odpojili
        c->state = CLIENT_MENU;
        c->paused = false;
        break;
    }

    if (c->state != CLIENT_IN_GAME) {
        break;
    }

    if (st.type == MSG_GAME_OVER) {
        printf("Koniec hry. Cas: %d\n", st.game_time);
        c->state = CLIENT_MENU;
        c->paused = false;
        break;
    }

    if (st.type == MSG_STATE) {
        draw_state(&st);
    }
  }

  return NULL;
}


