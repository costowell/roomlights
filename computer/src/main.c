#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "led.h"
#include "signal.h"

#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define PORTNAME "/dev/ttyACM0"

struct start_lightmode_arg {
  LightModeCallback lightmode;
  struct LightModeCommon *lmc;
};

static const LightModeCallback lightmodes[] = {
  lc_slow_clear,
  lc_volume,
  lc_volume_bright,
  lc_wave
};

void start_lightmode(struct start_lightmode_arg *args) {
  int status = args->lightmode(args->lmc);
  if (status != 0) {
    if (status == ENODEV) fprintf(stderr, "No such device. Device could have been unplugged.\n");
    else fprintf(stderr, "error: %s\n", strerror(status));
  }

  while(args->lmc->locked) {}
  args->lmc->locked = true;
  args->lmc->status = status;
  args->lmc->locked = false;
}

int main() {
  pthread_t p_lightmode_thread;
  pthread_t p_signal_thread;
  struct LightModeCommon lmc;
  struct start_lightmode_arg args;
  args.lmc = &lmc;

  memset(&p_lightmode_thread, 0, sizeof(pthread_t));
  memset(&p_signal_thread, 0, sizeof(pthread_t));

  lmc.terminate = false;
  lmc.serial = open_serial(PORTNAME);
  lmc.locked = false;
  lmc.status = 0;
  lmc.signal = 1;


  setup_signals(LEN(lightmodes));
  if (pthread_create(&p_signal_thread, NULL, (void *)(void *)(await_signals), &lmc)) {
    fprintf(stderr, "failed to start new thread!");
    exit(EXIT_FAILURE);
  }

  usleep(8000000);

  while (true) {
    if (lmc.status != 0) {
      printf("failed\n");
      break;
    }

    if (lmc.signal != -1) {
      if (p_lightmode_thread) {
        lmc.terminate = true;
        pthread_join(p_lightmode_thread, NULL);
        lmc.terminate = false;
      }

      args.lightmode = lightmodes[lmc.signal];

      if(pthread_create(&p_lightmode_thread, NULL, (void *)(void *)(start_lightmode), &args)) {
        fprintf(stderr, "failed to start new thread!"); 
        exit(EXIT_FAILURE);
      }
      while (lmc.locked) {}
      lmc.locked = true;
      lmc.signal = -1;
      lmc.locked = false;
    }
  }
}
