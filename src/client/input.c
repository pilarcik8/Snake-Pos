#include "input.h"
#include "client.h"

#include <unistd.h>
#include <termios.h>
#include <stdbool.h>

static struct termios g_old_term;

static void enable_raw_mode(void) {
  struct termios t;
  tcgetattr(STDIN_FILENO, &g_old_term);

  t = g_old_term;
  t.c_lflag &= (tcflag_t)~(ECHO | ICANON);
  t.c_cc[VMIN] = 0;
  t.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

static void disable_raw_mode(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &g_old_term);
}

static bool read_key(char *out) {
  char c;
  ssize_t n = read(STDIN_FILENO, &c, 1);
  if (n <= 0) return false;
  
  *out = c;
  return true;
}

static bool key_to_dir(char c, direction_t *dir_out) {
  if (c == 'w') { *dir_out = UP; return true; }
  if (c == 's') { *dir_out = DOWN; return true; }
  if (c == 'a') { *dir_out = LEFT; return true; }
  if (c == 'd') { *dir_out = RIGHT; return true; }
  return false;
}

void *input_thread_main(void *arg) {
  client_t *c = (client_t *)arg;

  enable_raw_mode();

  while (c->running) {
    char key;
    if (!read_key(&key)) {
      continue;
    }

    if (key == 'q') {
      c->running = false;
      break;
    }

    if (key == 'p') {
      client_message_t msg;
      msg.type = MSG_PAUSE;
      msg.direction = RIGHT; //nepouzivame na strane servera

      ipc_client_send(&c->ipc, &msg);
      c->paused = true;
      c->state = CLIENT_MENU;
      continue;
    }

    direction_t dir;
    if (key_to_dir(key, &dir)) {
      client_message_t msg;
      msg.type = MSG_INPUT;
      msg.direction = dir;

      ipc_client_send(&c->ipc, &msg);
    }
  }

  disable_raw_mode();
  return NULL;
}


