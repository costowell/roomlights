#pragma once

#include "led.h"

extern int signal_fd;

struct await_signals_args {
  struct LightModeCommon *lmc;
  unsigned int num_signals;
};

void term_handler();
void setup_signals(int lm_count);
void await_signals(struct await_signals_args *args);
