#include "world.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

bool world_in_bounds(const world_t *w, int x, int y) {
  return x >= 0 && x < w->width && y >= 0 && y < w->height;
}

bool world_is_free(const world_t *w, int x, int y) {
  return world_in_bounds(w, x, y) && w->cells[y][x] == CELL_EMPTY;
}

bool world_set_cell(world_t *w, int x, int y, cell_t c) {
  if (!world_in_bounds(w, x, y)) return false;
  w->cells[y][x] = c;
  return true;
}

static void world_clear(world_t *w) {
  for (int y = 0; y < w->height; y++) {
    for (int x = 0; x < w->width; x++) {
      w->cells[y][x] = CELL_EMPTY;
    }
  }
}

bool world_init(world_t *w, int width, int height) {
  if (!w) return false;
  if (width <= 0 || height <= 0) return false;
  if (width > MAX_WORLD_WIDTH || height > MAX_WORLD_HEIGHT) return false;

  w->width = width;
  w->height = height;
  world_clear(w);

  static int seeded = 0;
  if (!seeded) {
    seeded = 1;
    srand((unsigned)time(NULL));
  }

  return true;
}

static bool find_any_empty(const world_t *w, int *sx, int *sy) {
  for (int y = 0; y < w->height; y++) {
    for (int x = 0; x < w->width; x++) {
      if (w->cells[y][x] == CELL_EMPTY) {
        *sx = x; *sy = y;
          return true;
      }
    }
  }
  return false;
}

// Breadth-First Search (do šírky)
static bool all_empty_reachable(const world_t *w) {
  int sx, sy;

  if (!find_any_empty(w, &sx, &sy)) {
    return false;
  }

  int total = w->width * w->height;

  unsigned char visited[total];
  for (int i = 0; i < total; i++) {
    visited[i] = 0;
  }

  // front
  int qx[total];
  int qy[total];
  int qh = 0;   // head 
  int qt = 0;   // tail 

  visited[sy * w->width + sx] = 1;
  qx[qt] = sx;
  qy[qt] = sy;
  qt++;

  const int dx[4] = {  1, -1,  0,  0 };
  const int dy[4] = {  0,  0,  1, -1 };

  /* BFS */
  while (qh < qt) {
    int x = qx[qh];
    int y = qy[qh];
    qh++;

    for (int k = 0; k < 4; k++) {
      int nx = x + dx[k];
      int ny = y + dy[k];

      if (nx < 0) continue;
      if (ny < 0) continue;
      if (nx >= w->width) continue;
      if (ny >= w->height) continue;

      if (w->cells[ny][nx] != CELL_EMPTY) continue;

      int id = ny * w->width + nx;  // index -> 1D poľe

      if (visited[id] != 0) continue;

      // označ a vlož do fronty
      visited[id] = 1;
      qx[qt] = nx;
      qy[qt] = ny;
      qt++;
    }
  }
  
  // kontrola
  for (int y = 0; y < w->height; y++) {
    for (int x = 0; x < w->width; x++) {
      if (w->cells[y][x] == CELL_EMPTY) {
        int id = y * w->width + x;
        if (visited[id] == 0) {
          return false;
        }
      }
    }
  }

  return true;
}

// pridáva steny
static bool generate_obstacles_connected(world_t *w, int obstacle_percent) {
  if (!w) return false;

  if (obstacle_percent < 0) obstacle_percent = 0;
  if (obstacle_percent > 80) obstacle_percent = 80;

  int total = w->width * w->height;
  int target_walls = (total * obstacle_percent) / 100;

  world_clear(w);
  int placed = 0;

  const int max_failures_per_wall = 200;

  while (placed < target_walls) {
    int failures = 0;
    int placed_this_round = 0;

    // snaž sa nájsť jednu "dobrú" stenu
    while (failures < max_failures_per_wall) {
      int x = rand() % w->width;
      int y = rand() % w->height;

      if (w->cells[y][x] != CELL_EMPTY) {
        failures++;
        continue;
      }

      // dočasne polož stenu
      w->cells[y][x] = CELL_WALL;

      if (all_empty_reachable(w)) {
        // dobrá stena - necháme ju
        placed++;
        placed_this_round = 1;
        break;
      } else {
        // zlá stena - rollback
        w->cells[y][x] = CELL_EMPTY;
        failures++;
      }
    }

    if (!placed_this_round) {
      return true; 
    }
  }
  return true;
}

bool world_generate(world_t *w, world_type_t kind, int obstacle_percent) {
  if (!w) return false;

  if (kind == WORLD_NO_OBSTACLES) {
    world_clear(w);
    return true;
  }

  return generate_obstacles_connected(w, obstacle_percent);
}

// dlzka riadku
static int line_len_no_nl(const char *s) {
  int n = 0;
  while (s[n] && s[n] != '\n' && s[n] != '\r') n++;
  return n;
}

bool world_load_from_file(world_t *w, const char *path) {
  if (!w || !path) return false;

  FILE *f = fopen(path, "r");
  if (!f) {
    perror(path);
    return false;
  }

  char lines[MAX_WORLD_HEIGHT][MAX_WORLD_WIDTH + 4];
  int height = 0;
  int width = -1;

  while (height < MAX_WORLD_HEIGHT && fgets(lines[height], sizeof(lines[height]), f)) {
    int len = line_len_no_nl(lines[height]);
    if (len == 0) continue; // ignoruj prázdne riadky

    if (width == -1) width = len;
    if (len != width) { 
      fclose(f);
      return false; 
    }
    if (width > MAX_WORLD_WIDTH) { 
      fclose(f); 
      return false; 
    }

    height++;
  }

  fclose(f);

  if (width <= 0 || height <= 0) return false;
  if (!world_init(w, width, height)) return false;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      char c = lines[y][x];
      if (c == '#') w->cells[y][x] = CELL_WALL;
      else if (c == '.') w->cells[y][x] = CELL_EMPTY;
      else return false;
    }
  }
  return true;
}


