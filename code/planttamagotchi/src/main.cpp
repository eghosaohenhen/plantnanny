/*
 * Plant Tamagotchi — main app.
 * Emotions (blink faces) are driven by the plant's food/water state.
 *
 * Serial: 'f' = feed (food -> full), 'w' = water (water -> full)
 */
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include "sprite_player.h"
#include "emotions/animations.h"   // generated: ANIM_HAPPY / NEUTRAL / THIRSTY / HANGRY

#define TFT_DC  D6
#define TFT_CS  9
#define TFT_RST D7

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);
SpriteAnimator   face(tft, GC9A01A_BLACK);

// ---- plant state (0..100) ----
uint8_t water = 100;

// ---- mood mapping ----
enum Mood { HAPPY, NEUTRAL, THIRSTY, HANGRY };

Mood moodFromState() {
  if (water < 5) return HANGRY;    // needs feeding most urgently
  if (water < 25) return THIRSTY;
  if ( water > 70) return HAPPY;
  return NEUTRAL;
}

// Play the matching emotion. Called only when the mood actually changes —
// replaying every loop would keep restarting the blink timer so it'd never
// blink.
void applyMood(Mood m) {
  switch (m) {
    case HAPPY:   face.play(ANIM_HAPPY);   break;
    case NEUTRAL: face.play(ANIM_NEUTRAL); break;
    case THIRSTY: face.play(ANIM_THIRSTY); break;
    case HANGRY:  face.play(ANIM_HANGRY);  break;
  }
}

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.fillScreen(GC9A01A_BLACK);   // the ONLY full clear — never in loop()

  Mood m = moodFromState();
  applyMood(m);
}

void loop() {
  static Mood     current   = NEUTRAL;
  static uint32_t lastDecay = 0;

  // 1) let the current emotion animate (blink). Non-blocking.
  face.update();

  // 2) feed/water input
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'w') water = 100;
  }

  // 3) stats drift down slowly (tune the interval for your game pace)
  if (millis() - lastDecay > 3000) {
    lastDecay = millis();
    if (water) water--;
  }

  // 4) switch emotion ONLY when the mood changes
  Mood m = moodFromState();
  if (m != current) {
    current = m;
    applyMood(m);
  }

  // buzzer.update(); neopixel updates; buttons — all live here too, nothing blocks.
}