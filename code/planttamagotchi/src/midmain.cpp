// /*
//  * Plant Tamagotchi — subsystem bring-up harness
//  * Board: Seeed XIAO ESP32-C3
//  *
//  * Open the Serial Monitor at 115200 and send a single character:
//  *   1  -> display / sprite test
//  *   2  -> buzzer test
//  *   3  -> neopixel test
//  *   a  -> run all three in sequence
//  *
//  * Test each subsystem in ISOLATION first (1, then 2, then 3). Only after
//  * all three pass individually should you trust them running together.
//  */

// #include <Arduino.h>
// #include <SPI.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_GC9A01A.h>
// #include <Adafruit_NeoPixel.h>

// // ---------- Pin map ----------
// #define TFT_DC   D6
// #define TFT_CS   9
// #define TFT_RST  D7

// #define BUZZER_PIN   A3     // A3 / GPIO5, base of the 2N2222 via 1k
// #define NEO_PIN      D0    // GPIO2 — ASSUMPTION: change to your wired data pin
// #define NEO_COUNT    3

// Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);
// Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

// // Sprite header is optional so this compiles before you generate art.
// // Produce it with:  python3 png_to_rgb565.py plant.png --name plant_happy --out plant_happy.h
// #if __has_include("plant_happy.h")
// #include "plant_happy.h"
// #define HAVE_SPRITE 1
// #endif

// // Integer nearest-neighbor scale: each source pixel -> s*s block.
// // Same factor on both axes => never any non-uniform stretch, stays crisp.
// // Transparent pixels (mask bit 0) are skipped so the background shows through.
// void drawSpriteScaled(int16_t x, int16_t y, const uint16_t *data,
//                       const uint8_t *mask, int16_t w, int16_t h, uint8_t s) {
//   uint16_t bw = (w + 7) / 8;                  // mask row stride, byte-aligned
//   tft.startWrite();
//   for (int16_t sy = 0; sy < h; sy++) {
//     for (int16_t sx = 0; sx < w; sx++) {
//       if (mask) {
//         uint8_t byte = mask[sy * bw + sx / 8];
//         if (!(byte & (0x80 >> (sx & 7)))) continue;   // transparent -> skip
//       }
//       tft.writeFillRect(x + sx * s, y + sy * s, s, s, data[sy * w + sx]);
//     }
//   }
//   tft.endWrite();
// }

// // Largest integer factor that fits w*h inside the panel (uniform, no distortion).
// uint8_t fitScale(int16_t w, int16_t h) {
//   uint8_t s = min(tft.width() / w, tft.height() / h);
//   return s < 1 ? 1 : s;
// }

// // ---------- 1. Display / sprite ----------
// void testDisplay() {
//   Serial.println(F("[display] fill sweep..."));
//   tft.fillScreen(GC9A01A_RED);   delay(300);
//   tft.fillScreen(GC9A01A_GREEN); delay(300);
//   tft.fillScreen(GC9A01A_BLUE);  delay(300);
//   tft.fillScreen(GC9A01A_BLACK);

// #ifdef HAVE_SPRITE
//   // Uniform integer upscale (6x for 40x30 -> 240x180), then center.
//   uint8_t s = fitScale(PLANT_HAPPY_W, PLANT_HAPPY_H);
//   int16_t x = (tft.width()  - PLANT_HAPPY_W * s) / 2;
//   int16_t y = (tft.height() - PLANT_HAPPY_H * s) / 2;
//   Serial.printf("[display] sprite %dx%d @ %dx scale, top-left (%d,%d)\n",
//                 PLANT_HAPPY_W, PLANT_HAPPY_H, s, x, y);
//   drawSpriteScaled(x, y, plant_happy_data, plant_happy_mask,
//                    PLANT_HAPPY_W, PLANT_HAPPY_H, s);
// #else
//   Serial.println(F("[display] no plant_happy.h found — drawing placeholder."));
//   tft.fillRect(100, 105, 40, 30, GC9A01A_GREEN);
//   tft.setTextColor(GC9A01A_WHITE);
//   tft.setCursor(60, 150);
//   tft.print(F("gen sprite.h"));
// #endif
//   Serial.println(F("[display] PASS if you see color sweep + sprite/placeholder centered."));
// }

// // ---------- 2. Buzzer ----------
// // Drive the buzzer via LEDC directly instead of tone()/noTone(). On the
// // ESP32-C3, tone()'s noTone() can hit "LEDC is not initialized" — driving
// // LEDC ourselves avoids that and gives deterministic start/stop.
// #define BUZZER_CH   0     // LEDC channel (only used on core 2.x)

// void buzzerBegin() {
// #if ESP_ARDUINO_VERSION_MAJOR >= 3
//   ledcAttach(BUZZER_PIN, 2000, 10);      // pin, placeholder freq, 10-bit res
//   ledcWrite(BUZZER_PIN, 0);              // silent
// #else
//   ledcSetup(BUZZER_CH, 2000, 10);
//   ledcAttachPin(BUZZER_PIN, BUZZER_CH);
//   ledcWrite(BUZZER_CH, 0);
// #endif
// }

// void buzzerTone(uint16_t freq) {
// #if ESP_ARDUINO_VERSION_MAJOR >= 3
//   ledcWriteTone(BUZZER_PIN, freq);
// #else
//   ledcWriteTone(BUZZER_CH, freq);
// #endif
// }

// void buzzerOff() {
// #if ESP_ARDUINO_VERSION_MAJOR >= 3
//   ledcWriteTone(BUZZER_PIN, 0);
// #else
//   ledcWriteTone(BUZZER_CH, 0);
// #endif
// }

// void beep(uint16_t freq, uint16_t ms) {
//   buzzerTone(freq);
//   delay(ms);
//   buzzerOff();
// }

// void testBuzzer() {
//   Serial.println(F("[buzzer] ascending scale (should hear 8 rising notes)..."));
//   const uint16_t scale[] = {262, 294, 330, 349, 392, 440, 494, 523}; // C4..C5
//   for (uint16_t f : scale) { beep(f, 180); delay(40); }
//   delay(300);

//   Serial.println(F("[buzzer] tamagotchi chirp..."));
//   beep(880, 60); beep(1175, 60); beep(1568, 90);   // happy blip
//   delay(400);
//   beep(392, 250); beep(330, 350);                  // sad droop

//   buzzerOff(); // silent at idle (transistor off)
//   Serial.println(F("[buzzer] PASS if tones were audible and it is silent now."));
// }

// // ---------- 3. NeoPixels ----------
// void testNeopixel() {
//   Serial.println(F("[neopixel] walk each pixel white, one at a time..."));
//   for (int i = 0; i < NEO_COUNT; i++) {
//     strip.clear();
//     strip.setPixelColor(i, strip.Color(60, 60, 60)); // modest brightness
//     strip.show();
//     Serial.printf("[neopixel] pixel %d ON\n", i);
//     delay(500);
//   }

//   Serial.println(F("[neopixel] R / G / B on all three..."));
//   uint32_t rgb[] = {strip.Color(80,0,0), strip.Color(0,80,0), strip.Color(0,0,80)};
//   for (uint32_t c : rgb) {
//     for (int i = 0; i < NEO_COUNT; i++) strip.setPixelColor(i, c);
//     strip.show();
//     delay(500);
//   }

//   strip.clear();
//   strip.show();
//   Serial.println(F("[neopixel] PASS if each pixel lit in order, then R/G/B, then off."));
//   Serial.println(F("[neopixel] If colors are swapped, change NEO_GRB to NEO_RGB."));
// }

// void menu() {
//   Serial.println(F("\n=== select test: 1=display  2=buzzer  3=neopixel  a=all ==="));
// }

// void setup() {
//   Serial.begin(115200);
//   delay(300);
//   Serial.println(F("\nPlant Tamagotchi bring-up harness"));

//   tft.begin();
//   tft.fillScreen(GC9A01A_BLACK);

//   pinMode(BUZZER_PIN, OUTPUT);
//   buzzerBegin();

//   strip.begin();
//   strip.setBrightness(255); // per-pixel values already kept low above
//   strip.clear();
//   strip.show();

//   menu();
// }

// void loop() {
//   if (!Serial.available()) return;
//   char c = Serial.read();
//   switch (c) {
//     case '1': testDisplay();  break;
//     case '2': testBuzzer();   break;
//     case '3': testNeopixel(); break;
//     case 'a':
//       testDisplay(); delay(500);
//       testBuzzer();  delay(500);
//       testNeopixel();
//       break;
//     case '\n': case '\r': return; // ignore line endings
//     default: Serial.printf("unknown '%c'\n", c);
//   }
//   menu();
// }