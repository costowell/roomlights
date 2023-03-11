#pragma once

#include "led.h"

extern int signal_fd;

void term_handler();
void setup_signals();
void await_signals(struct LightModeCommon *lmc);
