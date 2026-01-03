#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"
#include "types.h"
#include <stdbool.h>

#include "../common/types.h"

// Typ spravy medzi klientom a serverom.
typedef enum {
    MSG_CONNECT,
    MSG_DISCONNECT,
    MSG_INPUT,
    MSG_STATE,
    MSG_PAUSE,
    MSG_GAME_OVER
} message_type_t;

// Sprava posielana z klienta na server.
typedef struct {
    message_type_t type;
    int player_id;
    direction_t direction;
} client_message_t;

// Stav jedneho hada v hre.
typedef struct {
    int player_id;
    bool alive;
    int length;
    int score;
    position_t body[MAX_SNAKE_LENGTH];
} snake_state_t;

// Sprava posielana zo servera klientom.
typedef struct {
    message_type_t type;

    int game_time;
    int player_count;

    snake_state_t snakes[MAX_PLAYERS];

    int fruit_count;
    position_t fruits[MAX_PLAYERS];
} server_message_t;

#endif

