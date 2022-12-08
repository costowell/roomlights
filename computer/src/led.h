#pragma once

#include <termios.h>
#include <math.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include "constants.h"


struct LightModeCommon {
  bool terminate;
  int serial;
};

struct RGB {
  uint8_t r;
  uint8_t b;
  uint8_t g;
};

typedef void (*LightModeCallback)(struct LightModeCommon*);
void lc_volume(struct LightModeCommon *lmc);
void lc_wave(struct LightModeCommon *lmc);
void lc_clear(struct LightModeCommon *lmc);


int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
int open_serial(char *portname);
