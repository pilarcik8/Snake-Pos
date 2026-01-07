#define _POSIX_C_SOURCE 199309 //pre čas

#include "server.h"
#include "game.h"
#include "collisions.h"
#include "fruit.h"

#include <stdlib.h>
#include <string.h>

// Globalny stav servera.
typedef struct {
  game_t game;
  world_t world;

  snake_t snakes[MAX_PLAYERS];
  bool slot_used[MAX_PLAYERS];

  int active_snakes;
  bool pass_through_edges_en;

  fruit_state_t fruit;

  pthread_mutex_t lock;
} global_t;

// Kontext pre vlakna.
typedef struct {
  server_t *server;
  ipc_server_t *ipc;
} server_context_t;

static global_t g; // Info hry

static server_t *g_server = NULL;
static ipc_server_t *g_ipc = NULL;

static void globals_init(void) {
  memset(&g, 0, sizeof(g));
  pthread_mutex_init(&g.lock, NULL);

  g.active_snakes = 0;
  g.pass_through_edges_en = true;

  for (int i = 0; i < MAX_PLAYERS; i++) {
    g.slot_used[i] = false;
    g.snakes[i].alive = false;
    g.snakes[i].player_id = -1;
    g.snakes[i].length = 0;
    g.snakes[i].score = 0;
  }

  fruit_init(&g.fruit);
}

static int find_slot_by_player_id_locked(int player_id) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (!g.slot_used[i]) continue;

    if (g.snakes[i].player_id == player_id) return i;
  }
  return -1;
}

static int find_free_slot_locked(void) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (!g.slot_used[i]) return i;
  }
  return -1;
}

static bool position_taken_by_any_snake_locked(int x, int y) {
  position_t p;
  p.x = x;
  p.y = y;

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (!g.slot_used[i]) continue;
    if (!g.snakes[i].alive) continue;

    for (int j = 0; j < g.snakes[i].length; j++) {
      if (g.snakes[i].body[j].x == p.x && g.snakes[i].body[j].y == p.y) return true;
    }
  }
  return false;
}

// vyberie nahodne volne policko pre spawn.
static bool pick_spawn_locked(position_t *out) {
  if (g.world.width <= 0 || g.world.height <= 0) return false;

  for (int tries = 0; tries < 2000; tries++) {
    int x = rand() % g.world.width;
    int y = rand() % g.world.height;

    if (!world_is_free(&g.world, x, y)) continue;
    if (position_taken_by_any_snake_locked(x, y)) continue;

    out->x = x;
    out->y = y;
    return true;
  }
  return false;
}

static void fruit_sync_locked(void) {
  fruit_sync(&g.world, g.snakes, g.slot_used, MAX_PLAYERS, g.active_snakes, &g.fruit);
}

static void handle_new_connection_locked(int player_id) {
  int slot = find_slot_by_player_id_locked(player_id);
  if (slot >= 0) return;

  slot = find_free_slot_locked();
  if (slot < 0) return;

  position_t spawn;
  if (!pick_spawn_locked(&spawn)) return;

  g.slot_used[slot] = true;
  snake_init(&g.snakes[slot], player_id, spawn);
  g.snakes[slot].alive = true;

  g.active_snakes++;
  fruit_sync_locked();
}

static void handle_disconnect_locked(int player_id) {
  int slot = find_slot_by_player_id_locked(player_id);
  if (slot < 0) return;

  if (g.snakes[slot].alive) {
    g.snakes[slot].alive = false;
    if (g.active_snakes > 0) g.active_snakes--;
  }

  g.slot_used[slot] = false;
  g.snakes[slot].player_id = -1;
  g.snakes[slot].length = 0;
  g.snakes[slot].score = 0;

  fruit_sync_locked();
}

// ovladanie hracom
static void handle_input_locked(int player_id, direction_t dir) {
  int slot = find_slot_by_player_id_locked(player_id);
  if (slot < 0) return;
  if (!g.snakes[slot].alive) return;

  snake_set_direction(&g.snakes[slot], dir);
}

// tvorba novej hry
static void start_new_game_locked(const game_config_t *cfg) {
  if (cfg->width <= 0 || cfg->height <= 0) return;

  // reset hry
  g.active_snakes = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    g.slot_used[i] = false;
    g.snakes[i].alive = false;
    g.snakes[i].player_id = -1;
    g.snakes[i].length = 0;
    g.snakes[i].score = 0;
  }
  fruit_init(&g.fruit);

  // nastav game
  g.game.mode = cfg->mode;
  g.game.time_limit_sec = cfg->time_limit_sec;
  game_init(&g.game);

  // nastav world
  world_init(&g.world, cfg->width, cfg->height);

  if (cfg->world_type == WORLD_NO_OBSTACLES) {
    world_generate(&g.world, WORLD_NO_OBSTACLES, 0);
    g.pass_through_edges_en = true;
  } 
  else if (cfg->world_type == WORLD_WITH_OBSTACLES) {
    world_generate(&g.world, WORLD_WITH_OBSTACLES, 5); // 5%
    g.pass_through_edges_en = false; // pri prekážkach typicky bez wrapu
  } else {
    world_generate(&g.world, WORLD_MAP_LOADED, 0); //NEIMPLEMENTOVANE
  }
  
  // pod g.lock
  if (g_server) g_server->game_running = true;

  if (g_ipc) {
    pthread_mutex_lock(&g_ipc->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
      int pid = g_ipc->client_player_id[i];
      if (pid != -1) handle_new_connection_locked(pid); // teraz už svet existuje -> spawn hada prejde
    }
    pthread_mutex_unlock(&g_ipc->lock);
  }
}

static void process_client_message_locked(int player_id, const client_message_t *msg) {
  int slot = find_slot_by_player_id_locked(player_id); // slot sa == indexu pola

  if (msg->type == MSG_DISCONNECT) {
    handle_disconnect_locked(player_id);
  }

  else if (msg->type == MSG_INPUT && !g.snakes[slot].paused) {
    handle_input_locked(player_id, msg->direction);
  }
  else if (msg->type == MSG_PAUSE) {
    if (slot >= 0 && g.snakes[slot].alive) {
      g.snakes[slot].paused = true;     // had stojí
      g.snakes[slot].resume_ms = 0;     // ešte nebeží “návratový” timer
    }
  }
  else if (msg->type == MSG_CREATE_GAME) {
    start_new_game_locked(&msg->cfg);
  }
  else if (msg->type == MSG_CONNECT) {
    if (slot >= 0) {
      // hráč sa vracia do svojej hry
      if (g.snakes[slot].alive && g.snakes[slot].paused) {
        g.snakes[slot].paused = false;
        g.snakes[slot].resume_ms = PAUSE_DELAY_SEC * 1000; // 3 sekundy
    }
    return;
  }

    // nový hráč
    if (g_server && g_server->game_running) {
      handle_new_connection_locked(player_id);
    }
  }
}

static void kill_snake_locked(int idx) {
  if (!g.slot_used[idx]) return;
  if (!g.snakes[idx].alive) return;

  g.snakes[idx].alive = false;
  if (g.active_snakes > 0) g.active_snakes--;

  fruit_sync_locked();
    // TODO: Urobit zapis bodov a casu hada po vypadnuti hraca.
}

// tvorba spravy pre render klienta
static void build_state_locked(server_message_t *out, int game_time) {
  out->type = MSG_STATE;
  out->game_time = game_time;
  out->player_count = g.active_snakes;

  // rozmery sveta pre render
  out->width  = g.world.width;
  out->height = g.world.height;

  memcpy(out->cells, g.world.cells, sizeof(g.world.cells)); // policka

  for (int i = 0; i < MAX_PLAYERS; i++) {
    out->snakes[i].player_id = g.snakes[i].player_id;
    out->snakes[i].length = g.snakes[i].length;
    out->snakes[i].alive = g.snakes[i].alive;
    out->snakes[i].score = g.snakes[i].score;

    for (int j = 0; j < g.snakes[i].length; j++) {
      out->snakes[i].body[j] = g.snakes[i].body[j];
    }
  }

  for (int i = 0; i < g.fruit.count; i++) {
    out->fruits[i] = g.fruit.pos[i];
  }
}

static void sleep_ms(int ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

static void *game_loop(void *arg) {
  server_context_t *ctx = (server_context_t *)arg;

  while (ctx->server->running) {
    sleep_ms(SERVER_TICK_MS); // TICK
    pthread_mutex_lock(&g.lock);
    
    // hrac este nevytvoril hru
     if (!ctx->server->game_running) {
        pthread_mutex_unlock(&g.lock);
        continue;
    }

    // koniec hry
    if (!game_update(&g.game, g.active_snakes, SERVER_TICK_MS)) {
      server_message_t end_msg;
      memset(&end_msg, 0, sizeof(end_msg));
      end_msg.type = MSG_GAME_OVER;
      end_msg.game_time = g.game.elapsed_time_sec;
      end_msg.player_count = g.active_snakes;

      pthread_mutex_unlock(&g.lock);

      // pošli koniec hry všetkým (ak nie sú klienti, nič sa nestane)
      ipc_server_send_state(ctx->ipc, &end_msg);

      // koniec hry = koniec procesu server
      ctx->server->game_running = false;
      ctx->server->running = false;

      // zavri server_fd, aby accept určite “spadol”
      close(ctx->ipc->server_fd);

      break;
    }

    // Normálny tick:
      // - pauza
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (!g.slot_used[i]) continue;
      if (!g.snakes[i].alive) continue;

      // ak je hráč v pauze -> had stojí
      if (g.snakes[i].paused) continue;

      // ak sa práve vrátil -> 3s stojí
      if (g.snakes[i].resume_ms > 0) {
        g.snakes[i].resume_ms -= SERVER_TICK_MS;
        if (g.snakes[i].resume_ms < 0) g.snakes[i].resume_ms = 0;
        continue;
      }
      
      // - pohyb hadov
      snake_move(&g.snakes[i], g.world.width, g.world.height, g.pass_through_edges_en);
    }

      // - kolízie (stena, vlastné telo, iný had)
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (!g.slot_used[i]) continue;
      if (!g.snakes[i].alive) continue;

      collision_result_t col = collisions_check_one(&g.world, g.snakes, MAX_PLAYERS, i);

      if (col.hit_self || col.hit_wall || col.hit_other) kill_snake_locked(i);
    }

      // - ovocie (ak niekto zjedol → grow + score + respawn ovocia)
    if (fruit_handle_eating(&g.world, g.snakes, g.slot_used, MAX_PLAYERS, &g.fruit)) fruit_sync_locked();

    // stav na odoslanie (čas = g.game.elapsed_time!)
    server_message_t state;
    memset(&state, 0, sizeof(state));
    build_state_locked(&state, g.game.elapsed_time_sec);

    // odomkni globálny stav a pošli update klientom
    pthread_mutex_unlock(&g.lock);
    ipc_server_send_state(ctx->ipc, &state);
  }

  pthread_mutex_lock(&g.lock);
  ctx->server->game_running = false;
  pthread_mutex_unlock(&g.lock);

  return NULL;
}

// komunikacia s klientom
static void *ipc_loop(void *arg) {
  server_context_t *ctx = (server_context_t *)arg;

  while (ctx->server->running) {
    // najprv spracuje spravy (aj DISCONNECT), aby sa slot neuvolnil neskoro.
    client_message_t msg;
    int slot = ipc_server_receive(ctx->ipc, &msg);

    if (slot >= 0) {
      int player_id = ctx->ipc->client_player_id[slot];

      pthread_mutex_lock(&g.lock);
      process_client_message_locked(player_id, &msg);
      pthread_mutex_unlock(&g.lock);

      if (msg.type == MSG_DISCONNECT) {
        pthread_mutex_lock(&ctx->ipc->lock);
        ctx->ipc->client_player_id[slot] = -1;
        pthread_mutex_unlock(&ctx->ipc->lock);
      }
    }
    
    // potom prijme nove pripojenie a vytvori hada
    int new_slot = ipc_server_accept(ctx->ipc);
    if (new_slot >= 0) {
      int new_player_id = ctx->ipc->client_player_id[new_slot];

      pthread_mutex_lock(&g.lock);
      handle_new_connection_locked(new_player_id);
      pthread_mutex_unlock(&g.lock);
    }
  }
  return NULL;
}

// inicializacia
void server_init(server_t *server) {
  server->running = true;
  server->game_running = false;

  globals_init();
}

// spustenie vlakien
void server_run(server_t *server, ipc_server_t *ipc) {
  static server_context_t ctx;
  ctx.server = server;
  ctx.ipc = ipc;

  //globalna, pointer
  g_server = server;
  g_ipc = ipc;

  server->ipc = ipc;

  pthread_create(&server->game_thread, NULL, game_loop, &ctx);
  pthread_create(&server->ipc_thread, NULL, ipc_loop, &ctx);
}

// vypnutie
void server_shutdown(server_t *server) {
  server->running = false;

  close(server->ipc->server_fd); // odblokuje accept()

  pthread_join(server->game_thread, NULL);
  pthread_join(server->ipc_thread, NULL);
}
