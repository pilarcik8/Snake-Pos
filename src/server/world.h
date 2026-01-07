#ifndef WORLD_H
#define WORLD_H

#include "../common/types.h"
#include "../common/config.h"

#include <stdbool.h>

typedef struct {
  int width;
  int height;
  cell_t cells[MAX_WORLD_HEIGHT][MAX_WORLD_WIDTH];
} world_t;

// inicializácia rozmerov + vyčistenie na CELL_EMPTY 
bool world_init(world_t *w, int width, int height);

// generovanie sveta (bez prekážok / s prekážkami) :warn("%s");
bool world_generate(world_t *w, world_type_t kind, int obstacle_percent);

// načítanie sveta zo súboru (ASCII mapa: '#' stena, '.' prázdne) 
bool world_load_from_file(world_t *w, const char *path);

// jednoduché operácie na bunkách 
bool world_in_bounds(const world_t *w, int x, int y);
bool world_is_free(const world_t *w, int x, int y);
bool world_set_cell(world_t *w, int x, int y, cell_t c);

#endif

