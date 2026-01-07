#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "config.h"
#include "types.h"
#include <stdbool.h>

// Typ spravy medzi klientom a serverom
typedef enum {
  MSG_CONNECT,
  MSG_DISCONNECT,
  MSG_INPUT,
  MSG_STATE,
  MSG_PAUSE,
  MSG_GAME_OVER,

  MSG_CREATE_GAME,   
} message_type_t;

typedef struct {
  game_mode_t mode;         // GAME_STANDARD / GAME_TIMED
  int time_limit_sec;       // len ak TIMED (inak 0)
  world_type_t world_type;  // bez/so prekážkami, z suboru
  int width;                // rozmery (ak nenačítavaš zo súboru)
  int height;
  bool allowed_multiplayer;
} game_config_t;

// Sprava posielana z klienta na server
typedef struct {
  message_type_t type;
  direction_t direction;    // používa sa len pri MSG_INPUT
  game_config_t cfg;        // používa sa len pri MSG_CREATE_GAME
} client_message_t;


// Stav jedneho hada v hre.
typedef struct {
  int player_id;
  bool alive;
  int length;
  int score;
  position_t body[MAX_SNAKE_LENGTH];
} snake_state_t;

// Sprava posielana zo servera klientom
typedef struct {
  message_type_t type;
  int game_time;
  int player_count;

  int width;
  int height;

  cell_t cells[MAX_WORLD_HEIGHT][MAX_WORLD_WIDTH];

  int assigned_player_id;
  
  snake_state_t snakes[MAX_PLAYERS];
  position_t fruits[MAX_PLAYERS];
} server_message_t;

#endif
