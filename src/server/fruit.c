#include "fruit.h"

#include <stdlib.h>
#include <wchar.h>

static bool pos_eq(position_t a, position_t b) {
    return a.x == b.x && a.y == b.y;
}

static bool fruit_pos_taken(const fruit_state_t *fs, position_t p) {
    for (int i = 0; i < fs->count; i++) {
        if (pos_eq(fs->pos[i], p)) return true;
    }
    return false;
}

static bool snake_occupies(const snake_t *s, position_t p) {
    if (!s->alive) return false;

    for (int i = 0; i < s->length; i++) {
        if (pos_eq(s->body[i], p)) return true;
    }
    return false;
}

static bool any_snake_occupies(const snake_t *snakes,
                               const bool *slot_used,
                               int snake_count,
                               position_t p) {
    for (int i = 0; i < snake_count; i++) {
        if (!slot_used[i]) continue;
        if (snake_occupies(&snakes[i], p)) return true;
    }
    return false;
}

static bool place_one_fruit(world_t *w,
                            const snake_t *snakes,
                            const bool *slot_used,
                            int snake_count,
                            fruit_state_t *fs) {
    for (int tries = 0; tries < 2000; tries++) {
        position_t p;
        p.x = rand() % w->width;
        p.y = rand() % w->height;

        if (w->cells[p.y][p.x] != CELL_EMPTY) continue;
        if (fruit_pos_taken(fs, p)) continue;
        if (any_snake_occupies(snakes, slot_used, snake_count, p)) continue;

        w->cells[p.y][p.x] = CELL_FRUIT;
        fs->pos[fs->count] = p;
        fs->count++;
        return true;
    }

    return false;
}

void fruit_init(fruit_state_t *fs) {
    fs->count = 0;
}

void fruit_sync(world_t *w,
                const snake_t *snakes,
                const bool *slot_used,
                int snake_count,
                int active_snakes,
                fruit_state_t *fs) {
    while (fs->count > active_snakes) {
        fs->count--;
        position_t p = fs->pos[fs->count];
        if (w->cells[p.y][p.x] == CELL_FRUIT) {
            w->cells[p.y][p.x] = CELL_EMPTY;
        }
    }

    while (fs->count < active_snakes) {
        if (!place_one_fruit(w, snakes, slot_used, snake_count, fs)) {
            break;
        }
    }
}

static void remove_fruit_at(world_t *w, fruit_state_t *fs, int idx) {
    position_t p = fs->pos[idx];
    if (w->cells[p.y][p.x] == CELL_FRUIT) {
        w->cells[p.y][p.x] = CELL_EMPTY;
    }

    fs->count--;
    fs->pos[idx] = fs->pos[fs->count];
}

bool fruit_handle_eating(world_t *w,
                         snake_t *snakes,
                         const bool *slot_used,
                         int snake_count,
                         fruit_state_t *fs) {
    bool eaten = false;

    for (int s = 0; s < snake_count; s++) {
        if (!slot_used[s]) continue;
        if (!snakes[s].alive) continue;

        position_t head = snakes[s].body[0];

        for (int i = 0; i < fs->count; i++) {
            if (!pos_eq(head, fs->pos[i])) continue;

            remove_fruit_at(w, fs, i);
            snake_grow(&snakes[s]);
            snakes[s].score += 1;
            eaten = true;
            break;
        }
    }

    return eaten;
}
