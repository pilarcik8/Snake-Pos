#include "client.h"

int main(void) {
  client_t client;
  client_init(&client);
  client_run(&client);  // zacne loop
  client_shutdown(&client);
  return 0;
}







