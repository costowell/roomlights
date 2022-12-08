#include <FastLED.h>

#define RGB_PIN         6
#define LED_NUM         300
#define LED_PER_SEG     5
#define SEGMENTS        LED_NUM / LED_PER_SEG
#define BRIGHTNESS      200
#define CHIP_SET        WS2812B
#define COLOR_CODE      GRB

CRGB leds[LED_NUM];
int time = 0;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIP_SET, RGB_PIN, COLOR_CODE>(leds, LED_NUM);
  // FastLED.setMaxRefreshRate(0, false);
  randomSeed(analogRead(0));
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  if (Serial.available() > 0) {
    for (int s = 0; s < SEGMENTS; s++) {
      byte buf[3];
      Serial.readBytes( (byte*)(&buf), 3);
      for (int i = 0; i < LED_PER_SEG; i++) {
        leds[(s * LED_PER_SEG)+i].setRGB(buf[0], buf[1], buf[2]);
      }
    }
    FastLED.show();
    Serial.write(1);
  }
}
