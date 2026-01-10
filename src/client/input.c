#include "input.h"
#include "client.h"

#include <termios.h>
#include <string.h>

static struct termios g_old_term; // pôvodné nastavenie terminálu

static void enable_raw_mode(void) {
  struct termios t;
  tcgetattr(STDIN_FILENO, &g_old_term);

  t = g_old_term;
  t.c_lflag &= (tcflag_t)~(ECHO | ICANON); // stlačanie klaves sa nezobrazuje
  t.c_cc[VMIN] = 0; // ter. nemusí čakať na minimálny počet znakov, stačí 0
  t.c_cc[VTIME] = 1; // ter. čaká 100ms na input

  tcsetattr(STDIN_FILENO, TCSANOW, &t); // aplikuje nastavania
}

static void disable_raw_mode(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &g_old_term);
}

// return == jeden znak z klavesnice
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
    if (!read_key(&key)) continue;
    if (c->paused) continue;

    if (key == 'p') {
      client_message_t msg;
      msg.type = MSG_PAUSE;
      msg.direction = RIGHT; //nevyuzijem na strane servera

      ipc_client_send(&c->ipc, &msg);
      c->paused = true;
      c->state = CLIENT_MENU;
      break;  // join() v main skončí
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


