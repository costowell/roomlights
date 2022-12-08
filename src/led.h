#pragma once

#include <termios.h>
#include <math.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>

#define LED_NUM 300
#define LED_PER_SEG 5
#define LED_SEGMENTS (LED_NUM / LED_PER_SEG)

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


int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
int open_serial(char *portname);
