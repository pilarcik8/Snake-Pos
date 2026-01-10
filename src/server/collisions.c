#include "collisions.h"

#include <stddef.h>

static bool positions_equal(position_t a, position_t b) {
  return a.x == b.x && a.y == b.y;
}

static bool hit_wall(const world_t *w, position_t head) {
  if (!world_in_bounds(w, head.x, head.y)) return true; // moze tu byt lebo iba bezstenna mapa nema wrap

  return w->cells[head.y][head.x] == CELL_WALL;
}

static bool hit_other_snake(const snake_t *snakes, int snake_count, int idx) {
  position_t head = snakes[idx].body[0];

  for (int s = 0; s < snake_count; s++) {
    if (s == idx) continue;
    if (!snakes[s].alive) continue;

    for (int i = 0; i < snakes[s].length; i++) {
      if (positions_equal(head, snakes[s].body[i])) return true;
    }
  }
  return false;
}

collision_result_t collisions_check_one(const world_t *w, const snake_t *snakes, int snake_count, int idx) {
  collision_result_t r;
  r.hit_wall = false;
  r.hit_self = false;
  r.hit_other = false;

  if (!snakes[idx].alive) return r;
  if (snakes[idx].length <= 0) return r;

  position_t head = snakes[idx].body[0];

  r.hit_self = snake_self_collision(&snakes[idx]);
  r.hit_other = hit_other_snake(snakes, snake_count, idx);
  
  if (w != NULL) { // existuje svet?
    r.hit_wall = hit_wall(w, head);
  }
  return r;
}

