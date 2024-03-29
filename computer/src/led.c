#include "led.h"
#include "common.h"
#include "pulse.h"
#include "cavacore.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

uint8_t rgb_buf[LED_SEG_PER_REP * 3];

/* Utility Functions */
struct RGB hsv_to_rgb(float H, float S, float V) {
  float s = S/100;
  float v = V/100;
  float C = s*v;
  float X = C*(1-fabs(fmod(H/60.0, 2)-1));
  float m = v-C;
  float r,g,b;
  if(H >= 0 && H < 60){
      r = C,g = X,b = 0;
  }
  else if(H >= 60 && H < 120){
      r = X,g = C,b = 0;
  }
  else if(H >= 120 && H < 180){
      r = 0,g = C,b = X;
  }
  else if(H >= 180 && H < 240){
      r = 0,g = X,b = C;
  }
  else if(H >= 240 && H < 300){
      r = X,g = 0,b = C;
  }
  else{
      r = C,g = 0,b = X;
  }
  struct RGB rgb = {
    .r = (uint8_t)((r+m)*255),
    .g = (uint8_t)((g+m)*255),
    .b = (uint8_t)((b+m)*255)
  };
  return rgb;
}

bool is_valid_fd(int fd) {
  return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

bool write_leds(int fd) {
  if (!is_valid_fd(fd)) {
    errno = ENODEV;
    return false;
  }
  int status = write(fd, rgb_buf, sizeof(rgb_buf));

  if (status == -1) return false;

  char buf[1];
  while(read(fd, buf, sizeof(buf)) == -1) {}
  return true;
}

/* Light Controlling Functions */
int lc_clear(struct LightModeCommon *lmc) {
  memset(&rgb_buf, 0, 3 * LED_SEG_PER_REP);
  if (!write_leds(lmc->serial)) return errno;
  while(!lmc->terminate) usleep(100000);
  return 0;
}

int lc_slow_clear(struct LightModeCommon *lmc) {
  for (int n = 0; n < (LED_SEG_PER_REP / 2); n++) {
    for (int i = 0; i < 3; i++) {
      rgb_buf[(n * 3) + i] = 0;
      rgb_buf[(LED_SEG_PER_REP * 3) - ((n * 3) + i) - 1] = 0;
    }
    if(!write_leds(lmc->serial)) return errno;
  }
  return lc_clear(lmc);
}

int lc_wave(struct LightModeCommon *lmc) {
  int wave_position = 0;
  const int end_pos = LED_NUM;
  while(!lmc->terminate) {
    int led_position = wave_position * 3;
    if (wave_position != 0) {
      rgb_buf[led_position - 3] = 0;
      rgb_buf[led_position - 2] = 0;
      rgb_buf[led_position - 1] = 0;
    } else {
      rgb_buf[end_pos-1] = 0;
      rgb_buf[end_pos-2] = 0;
      rgb_buf[end_pos-3] = 0;
    }
    rgb_buf[led_position] = 0;
    rgb_buf[led_position + 1] = 0;
    rgb_buf[led_position + 2] = 255;
    wave_position++;
    if (wave_position == LED_SEG_PER_REP) {
      wave_position = 0;
    }
    if(!write_leds(lmc->serial)) return errno;
  }
  return 0;
}

int _lc_volume(struct LightModeCommon *lmc, int power, double noise_reduction) {
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

  int output_channels = 2;
  double *cava_out;
  struct cava_plan *plan = cava_init(LED_SEG_PER_REP / output_channels, audio.rate, audio.channels, 1, noise_reduction, 50, 10000);

  if (plan->status == -1) {
    fprintf(stderr, "Error inititalizing cava . %s", plan->error_message);
    exit(EXIT_FAILURE);
  }
  cava_out = (double *)malloc(LED_SEG_PER_REP / output_channels * audio.channels * sizeof(double));
  memset(cava_out, 0, LED_SEG_PER_REP/output_channels * audio.channels * sizeof(double));

  int sleep_counter = 0;
  bool silence = true;
  while (!lmc->terminate) {
    clock_t start, end;

    start = clock();

    // check if audio input is present
    silence = true;

    for (int n = 0; n < (audio.input_buffer_size * 2); n++) {
      if (audio.cava_in[n] && n != 2048) {
        silence = false;
        break;
      }
    }

    // increment sleep counter when silent, reset if not
    if (silence && sleep_counter <= 50)
      sleep_counter++;
    else if (!silence)
      sleep_counter = 0;
    else { // runs when sleep_counter > 1000
      usleep(1000000);
      continue;
    }

    pthread_mutex_lock(&audio.lock);
    cava_execute(audio.cava_in, audio.samples_counter, cava_out, plan);
    if (audio.samples_counter > 0) {
        audio.samples_counter = 0;
    }
    pthread_mutex_unlock(&audio.lock);

    int hue_interval = (360/LED_SEG_PER_REP) * 2;
    for (int n = 0; n < LED_SEG_PER_REP / 2; n++) {
      struct RGB rgb = hsv_to_rgb(hue_interval * n, 100, pow(cava_out[n], power) * 100);
      int pos = n * 3;
      rgb_buf[pos]   = rgb.r;
      rgb_buf[pos+1] = rgb.g;
      rgb_buf[pos+2] = rgb.b;
    }
    for (int n = LED_SEG_PER_REP / 2; n < LED_SEG_PER_REP; n++) {
      struct RGB rgb = hsv_to_rgb(hue_interval * (LED_SEG_PER_REP - n), 100, pow(cava_out[LED_SEG_PER_REP - n - 1], power) * 100);
      int pos = n * 3;
      rgb_buf[pos]   = rgb.r;
      rgb_buf[pos+1] = rgb.g;
      rgb_buf[pos+2] = rgb.b;
    }

    // for (int i = 0; i < 3 * LED_SEGMENTS; i+=3) {
    //   printf("\x1b[48;2;%u;%u;%um ", rgb_buf[i], rgb_buf[i+1], rgb_buf[i+2]);
    // }
    // printf("\x1b[0G");
    // fflush(stdout);
    if (!write_leds(lmc->serial)) return errno;
    end = clock();
    // printf("%f ms\n", (double)(end - start) / CLOCKS_PER_SEC * 1000);
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

  return 0;
}

int lc_volume(struct LightModeCommon *lmc) {
  return _lc_volume(lmc, 2, 0.77);
}

int lc_volume_bright(struct LightModeCommon *lmc) {
  return _lc_volume(lmc, 3, 0.50);
}

/* Serial connecton related */
int set_interface_attribs (int fd, int speed, int parity) {
  struct termios tty;
  if (tcgetattr (fd, &tty) != 0) {
    fprintf (stderr, "error %d from tcgetattr", errno);
    return -1;
  }

  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;         // disable break processing
  tty.c_lflag = 0;                // no signaling chars, no echo,
                                  // no canonical processing
  tty.c_oflag = 0;                // no remapping, no delays
  tty.c_cc[VMIN]  = 0;            // read doesn't block
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr (fd, TCSANOW, &tty) != 0) {
          fprintf(stderr, "error %d from tcsetattr", errno);
          return -1;
  }
  return 0;
}

void set_blocking (int fd, int should_block) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    fprintf(stderr, "error %d from tggetattr", errno);
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    fprintf(stderr, "error %d setting term attributes", errno);
}


int open_serial(char *portname) {
  int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    fprintf(stderr, "error %d opening %s: %s", errno, portname, strerror (errno));
    exit(EXIT_FAILURE);
  }

  set_interface_attribs(fd, B115200, 0);
  set_blocking(fd, 0);
  return fd;
}
