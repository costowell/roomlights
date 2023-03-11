#pragma once

#include <termios.h>
#include <math.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include "constants.h"


struct LightModeCommon {
  bool locked;
  bool terminate;
  int serial;
  int status;
  int signal;
};

struct RGB {
  uint8_t r;
  uint8_t b;
  uint8_t g;
};

typedef int (*LightModeCallback)(struct LightModeCommon*);
int lc_volume(struct LightModeCommon *lmc);
int lc_volume_bright(struct LightModeCommon *lmc);
int lc_wave(struct LightModeCommon *lmc);
int lc_clear(struct LightModeCommon *lmc);
int lc_slow_clear(struct LightModeCommon *lmc);

int _lc_volume(struct LightModeCommon *lmc, int power, double noise_reduction);
int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
int open_serial(char *portname);
bool is_valid_fd(int fd);
bool write_leds(int fd);
