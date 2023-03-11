#include "signal.h"

#include <unistd.h>
#include <sys/signalfd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int signal_fd;

void term_handler() {
  exit(EXIT_FAILURE);
}

void await_signals(struct LightModeCommon *lmc) {
  while(true) {
    if (lmc->status != 0) break;
    struct signalfd_siginfo info;
    read(signal_fd, &info, sizeof(info));
    int signal = info.ssi_signo - SIGRTMIN;

    while (lmc->locked) {}
    lmc->locked = true;
    lmc->signal = signal;
    lmc->locked = false;
  }
}

void setup_signals(int lm_count) {
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
	for (int i = 0; i < lm_count; i++)
		sigaddset(&handled_signals, SIGRTMIN + i);
	signal_fd = signalfd(-1, &handled_signals, 0);
}

