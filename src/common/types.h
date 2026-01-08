#ifndef TYPES_H
#define TYPES_H

typedef enum {
  UP,
  DOWN,
  LEFT,
  RIGHT
} direction_t;

typedef enum {
  GAME_STANDARD = 1,
  GAME_TIMED = 2
} game_mode_t;

typedef enum {
  CELL_EMPTY,
  CELL_WALL,
  CELL_FRUIT,
  CELL_SNAKE
} cell_t;

typedef struct {
  int x;
  int y;
} position_t;

typedef enum {
  WORLD_NO_OBSTACLES = 1,
  WORLD_WITH_OBSTACLES = 2,
  WORLD_MAP_LOADED = 3
} world_type_t;

#endif

