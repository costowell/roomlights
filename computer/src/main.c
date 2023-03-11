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

struct start_lightmode_args {
  LightModeCallback lightmode;
  struct LightModeCommon *lmc;
};

static const LightModeCallback lightmodes[] = {
  lc_slow_clear,
  lc_volume,
  lc_volume_bright,
  lc_wave
};

void start_lightmode(struct start_lightmode_args *args) {
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

  memset(&p_lightmode_thread, 0, sizeof(pthread_t));
  memset(&p_signal_thread, 0, sizeof(pthread_t));

  lmc.terminate = false;
  lmc.serial = open_serial(PORTNAME);
  lmc.locked = false;
  lmc.status = 0;
  lmc.signal = 1;

  struct start_lightmode_args lm_args;
  struct await_signals_args sig_args;
  memset(&sig_args, 0, sizeof(struct await_signals_args));
  memset(&lm_args, 0, sizeof(struct start_lightmode_args));
  lm_args.lmc = &lmc;
  lm_args.lightmode = NULL;
  sig_args.lmc = &lmc;
  sig_args.num_signals = LEN(lightmodes);

  setup_signals(LEN(lightmodes));
  if (pthread_create(&p_signal_thread, NULL, (void *)(void *)(await_signals), &sig_args)) {
    fprintf(stderr, "failed to start new thread!");
    exit(EXIT_FAILURE);
  }

  printf("Initialized!\nWaiting for arduino to be ready...");
  fflush(stdout);
  usleep(4000000);
  printf(" Done!\n");

  while (true) {
    if (lmc.status != 0) {
      printf("Received error code %d from lightmode: %s\nExiting...\n", lmc.status, strerror(lmc.status));
      break;
    }

    if (lmc.signal != -1) {
      if (p_lightmode_thread) {
        while (lmc.locked) { }
        lmc.locked = true;
        lmc.terminate = true;
        lmc.locked = false;
        pthread_join(p_lightmode_thread, NULL);
        while(lmc.locked) { }
        lmc.locked = true;
        lmc.terminate = false;
        lmc.locked = false;
      }

      printf("Switching to mode %d\n", lmc.signal);

      // memset(&args, 0, sizeof(struct start_lightmode_arg));
      lm_args.lightmode = lightmodes[lmc.signal];

      if(pthread_create(&p_lightmode_thread, NULL, (void *)(void *)(start_lightmode), &lm_args)) {
        fprintf(stderr, "failed to start new thread!"); 
        exit(EXIT_FAILURE);
      }
      while (lmc.locked) { }
      lmc.locked = true;
      lmc.signal = -1;
      lmc.locked = false;
    }
  }
}
