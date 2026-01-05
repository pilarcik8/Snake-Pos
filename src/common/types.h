#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction_t;

typedef enum {
    GAME_STANDARD,
    GAME_TIMED
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
    WORLD_NO_OBSTACLES = 0,
    WORLD_WITH_OBSTACLES = 1
} world_type_t;

#endif

