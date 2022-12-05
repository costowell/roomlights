#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "pulse.h"
#include "common.h"
#include "cavacore.h"

int main() {
  while(true) {
    struct audio_data audio;
    memset(&audio, 0, sizeof(audio));

    // audio.source = "default";

    audio.format = 16;
    audio.rate = 44100;
    audio.samples_counter = 0;
    audio.channels = 2;
    audio.IEEE_FLOAT = 0;

    audio.input_buffer_size = BUFFER_SIZE * audio.channels;
    audio.cava_buffer_size = audio.input_buffer_size * 8;
    audio.cava_in = (double *)malloc(audio.cava_buffer_size * sizeof(double));
    memset(audio.cava_in, 0, sizeof(int) * audio.cava_buffer_size);

    audio.terminate = 0;

    pthread_t p_thread;

    int thr_id;

    pthread_mutex_init(&audio.lock, NULL);

    // Pulse
    getPulseDefaultSink((void *)&audio);
    thr_id = pthread_create(&p_thread, NULL, input_pulse, (void *)&audio);

    int number_of_bars = 60;
    int output_channels = 2;
    double *cava_out;
    struct cava_plan *plan = cava_init(number_of_bars / output_channels, audio.rate, audio.channels, 1, 0.77, 50, 10000);

    if (plan->status == -1) {
      fprintf(stderr, "Error inititalizing cava . %s", plan->error_message);
      exit(EXIT_FAILURE);
    }
    cava_out = (double *)malloc(number_of_bars / output_channels * audio.channels * sizeof(double));
    memset(cava_out, 0, number_of_bars/output_channels * audio.channels * sizeof(double));

    int sleep_counter = 0;
    bool silence = true;
    struct timespec sleep_mode_timer = { .tv_sec = 1, .tv_nsec = 0 };
    struct timespec loop_timer = { .tv_sec = 0, .tv_nsec = 1000000 };
    while (true) {

      // check if audio input is present
      silence = true;

      for (int n = 0; n < (audio.input_buffer_size * 2); n++) {
        if (audio.cava_in[n] && n != 2048) {
          silence = false;
          break;
        }
      }

      // printf("%u", sleep_counter);

      // increment sleep counter when silent, reset if not
      if (silence && sleep_counter <= 1000)
        sleep_counter++;
      else if (!silence)
        sleep_counter = 0;
      else { // runs when sleep_counter > 1000
        nanosleep(&sleep_mode_timer, NULL);
        continue;
      }

      pthread_mutex_lock(&audio.lock);
      cava_execute(audio.cava_in, audio.samples_counter, cava_out, plan);
      if (audio.samples_counter > 0) {
          audio.samples_counter = 0;
      }
      pthread_mutex_unlock(&audio.lock);


      for (int n = 0; n < number_of_bars / 2; n++) {
        printf("%-4d", (int)( cava_out[n] * 100 ));
      }
      printf("\x1b[0E");
      fflush(stdout);

      nanosleep(&loop_timer, NULL);
    }

    
    pthread_mutex_lock(&audio.lock);
    audio.terminate = 1;
    pthread_mutex_unlock(&audio.lock);
    pthread_join(p_thread, NULL);
    cava_destroy(plan);

    free(plan);
    free(cava_out);
    free(audio.source);
    free(audio.cava_in);
  }
}
