#include "world.h"
#include <stdlib.h>
#include <string.h>
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

// dosiahnuteľnosť všetkých voľných polí pri náhodných prekážkach

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

static bool all_empty_reachable(const world_t *w) {
  int sx, sy;
  if (!find_any_empty(w, &sx, &sy)) return false;

  const int maxn = MAX_WORLD_WIDTH * MAX_WORLD_HEIGHT;
  unsigned char visited[maxn];
  memset(visited, 0, sizeof(visited));

  int qx[maxn];
  int qy[maxn];
  int qh = 0, qt = 0;

  #define IDX(xx, yy) ((yy) * w->width + (xx))

  visited[IDX(sx, sy)] = 1;
  qx[qt] = sx; qy[qt] = sy; qt++;

  static const int dx[4] = { 1, -1, 0, 0 };
  static const int dy[4] = { 0, 0, 1, -1 };

  while (qh < qt) {
    int x = qx[qh];
    int y = qy[qh];
    qh++;

    for (int k = 0; k < 4; k++) {
      int nx = x + dx[k];
      int ny = y + dy[k];
      if (!world_in_bounds(w, nx, ny)) continue;
      if (w->cells[ny][nx] != CELL_EMPTY) continue;

      int id = IDX(nx, ny);
      if (!visited[id]) {
        visited[id] = 1;
        qx[qt] = nx; qy[qt] = ny; qt++;
      }
    }
  }

  for (int y = 0; y < w->height; y++) {
    for (int x = 0; x < w->width; x++) {
      if (w->cells[y][x] == CELL_EMPTY) {
        if (!visited[IDX(x, y)]) return false;
      }
    }
  }

  #undef IDX
  return true;
}

static bool generate_obstacles_connected(world_t *w, int obstacle_percent) {
  if (obstacle_percent < 0) obstacle_percent = 0;
  if (obstacle_percent > 80) obstacle_percent = 80;

  const int total = w->width * w->height;
  const int target_walls = (total * obstacle_percent) / 100;

  const int attempts = 200;
  for (int a = 0; a < attempts; a++) {
    world_clear(w);

    int placed = 0;
    while (placed < target_walls) {
      int x = rand() % w->width;
      int y = rand() % w->height;
      if (w->cells[y][x] == CELL_EMPTY) {
        w->cells[y][x] = CELL_WALL;
        placed++;
      }
    }

    if (all_empty_reachable(w)) {
      return true;
    }
  }
  return false;
}

bool world_generate(world_t *w, world_type_t kind, int obstacle_percent) {
    if (!w) return false;

    if (kind == WORLD_NO_OBSTACLES) {
        world_clear(w);
        return true;
    }

    return generate_obstacles_connected(w, obstacle_percent);
}

/* ---- Povinné: načítanie sveta zo súboru ---- */
/* ASCII: '#' = prekážka, '.' = voľné */

static int line_len_no_nl(const char *s) {
    int n = 0;
    while (s[n] && s[n] != '\n' && s[n] != '\r') n++;
    return n;
}

bool world_load_from_file(world_t *w, const char *path) {
    if (!w || !path) return false;

    FILE *f = fopen(path, "r");
    if (!f) return false;

    char lines[MAX_WORLD_HEIGHT][MAX_WORLD_WIDTH + 4];
    int height = 0;
    int width = -1;

    while (height < MAX_WORLD_HEIGHT && fgets(lines[height], sizeof(lines[height]), f)) {
        int len = line_len_no_nl(lines[height]);
        if (len == 0) continue; // ignoruj prázdne riadky

        if (width == -1) width = len;
        if (len != width) { fclose(f); return false; }
        if (width > MAX_WORLD_WIDTH) { fclose(f); return false; }

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


