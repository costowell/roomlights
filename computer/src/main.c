#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/signalfd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "led.h"

#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define PORTNAME "/dev/ttyACM0"

static const LightModeCallback lightmodes[] = {
  lc_clear,
  lc_volume,
  lc_volume_bright,
  lc_wave
};
static int signal_fd;

void term_handler() {
  exit(EXIT_FAILURE);
}

void setup_signals() {
	// Ignore all realtime signals
	sigset_t ignored_signals;
	sigemptyset(&ignored_signals);
	for (int i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaddset(&ignored_signals, i);
	sigprocmask(SIG_BLOCK, &ignored_signals, NULL);

	// Handle termination signals
	signal(SIGINT, term_handler);
	signal(SIGTERM, term_handler);

	// Avoid zombie subprocesses
	struct sigaction sa;
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGCHLD, &sa, 0);

	// Handle block update signals
	sigset_t handled_signals;
	sigemptyset(&handled_signals);
	for (int i = 0; i < LEN(lightmodes); i++)
		sigaddset(&handled_signals, SIGRTMIN + i);
	signal_fd = signalfd(-1, &handled_signals, 0);
}

int main() {
  pthread_t p_thread;
  struct LightModeCommon lmc;

  memset(&p_thread, 0, sizeof(pthread_t));

  lmc.terminate = false;
  lmc.serial = open_serial(PORTNAME);

  setup_signals();
  usleep(8000000);

  while (true) {
    struct signalfd_siginfo info;
    read(signal_fd, &info, sizeof(info));
    int signal = info.ssi_signo - SIGRTMIN;
    if (signal < 0 || signal >= LEN(lightmodes))
      break;
    
    if (p_thread) {
      lmc.terminate = true;
      pthread_join(p_thread, NULL);
      lmc.terminate = false;
    }

    if(pthread_create(&p_thread, NULL, lightmodes[signal], &lmc)) {
      fprintf(stderr, "failed to start new thread!"); 
      exit(EXIT_FAILURE);
    }
  }
}
