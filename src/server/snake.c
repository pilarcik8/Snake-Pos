#include "snake.h"

static bool is_opposite(direction_t a, direction_t b) {
  if (a == UP && b == DOWN) return true;
  if (a == DOWN && b == UP) return true;
  if (a == LEFT && b == RIGHT) return true;
  if (a == RIGHT && b == LEFT) return true;
  return false;
}

void snake_init(snake_t *s, int player_id, position_t start) {
  s->player_id = player_id;
  s->length = 1;
  s->body[0] = start;
  s->direction = RIGHT;
  s->alive = true;
  s->score = 0;

  s->paused = false;
  s->resume_ms = 0;
}

void snake_set_direction(snake_t *s, direction_t dir) {
  if (!s->alive) return;
  if (s->length > 1 && is_opposite(s->direction, dir)) return;
  if (s->dir_locked) return;

  s->direction = dir;
  s->dir_locked = true;
}

position_t snake_next_head(const snake_t *s) {
  position_t head = s->body[0];

  if (s->direction == UP) head.y -= 1;
  else if (s->direction == DOWN) head.y += 1;
  else if (s->direction == LEFT) head.x -= 1;
  else if (s->direction == RIGHT) head.x += 1;

  return head;
}

static position_t apply_wrap(position_t p, int w, int h) {
  if (p.x < 0) p.x = w - 1;
  else if (p.x >= w) p.x = 0;

  if (p.y < 0) p.y = h - 1;
  else if (p.y >= h) p.y = 0;

  return p;
}

void snake_move(snake_t *s, int world_w, int world_h, bool wrap) {
  s->dir_locked = false;

  if (!s->alive) return;
  if (s->length <= 0) return;

  position_t new_head = snake_next_head(s);
  if (wrap) {
    new_head = apply_wrap(new_head, world_w, world_h);
  }

  for (int i = s->length - 1; i > 0; i--) {
    s->body[i] = s->body[i - 1];
  }
  s->body[0] = new_head;
}

void snake_grow(snake_t *s) {
  if (!s->alive) return;
  if (s->length >= MAX_SNAKE_LENGTH) return;

  s->body[s->length] = s->body[s->length - 1];
  s->length += 1;
}

bool snake_self_collision(const snake_t *s) {
  if (!s->alive) return false;
  if (s->length < 4) return false;

  position_t head = s->body[0];
  for (int i = 1; i < s->length; i++) {
    if (s->body[i].x == head.x && s->body[i].y == head.y) {
      return true;
    }
  }
  return false;
}


