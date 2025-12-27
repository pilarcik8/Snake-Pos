#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "types.h"
#include "config.h"

typedef enum {
    MSG_CONNECT,
    MSG_DISCONNECT,
    MSG_INPUT,
    MSG_PAUSE,
    MSG_STATE,
    MSG_GAME_OVER
} message_type_t;

/* správa od klienta */
typedef struct {
    message_type_t type;
    int player_id;
    direction_t direction;
} client_message_t;

/* stav jedného hada */
typedef struct {
    int player_id;
    int length;
    position_t body[MAX_SNAKE_LENGTH];
    bool alive;
    int score;
} snake_state_t;

/* správa od servera */
typedef struct {
    message_type_t type;
    int game_time;
    int player_count;
    snake_state_t snakes[MAX_PLAYERS];
} server_message_t;

#endif

