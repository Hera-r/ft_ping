#include "ft_ping.h"

volatile sig_atomic_t g_stop = 0;

void handle_sigint(int sig) {
  (void)sig;
  g_stop = 1;
}

void setup_signals(void) {
  struct sigaction sa;

  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
}
