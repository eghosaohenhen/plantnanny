#pragma once
#include <Arduino.h>
#include <Adafruit_GC9A01A.h>

// Best RNG source: ESP32 has a hardware RNG (no seeding needed). Fall back
// to Arduino random() elsewhere (remember randomSeed() then).
#if __has_include(<esp_random.h>)
#include <esp_random.h>
#define RAND32() esp_random()
#else
#define RAND32() ((uint32_t)random())
#endif

// A single frame: RGB565 pixels + optional 1-bpp transparency mask.
// mask == nullptr means fully opaque. Produced by png_to_rgb565.py.
struct Frame {
  const uint16_t *data;
  const uint8_t  *mask;   // nullptr = opaque
  uint16_t        w, h;
};

// How an Animation's frames should be interpreted.
enum AnimKind : uint8_t {
  ANIM_SEQUENCE = 0,  // cycle frames every frame_ms (the default)
  ANIM_BLINK    = 1,  // frames[0]=eyes open, frames[1]=eyes closed; auto-blink
};

// One tagged struct covers both a normal frame sequence AND a blink "emotion".
// Sequence anims only use the first four fields; blink fields default to 0.
struct Animation {
  const Frame *frames;
  uint8_t      count;
  uint16_t     frame_ms;        // SEQUENCE: per-frame duration
  bool         loop;

  AnimKind     kind;            // SEQUENCE (0) or BLINK
  uint16_t     blink_close_ms;  // BLINK: how long the eyes stay shut
  uint16_t     blink_min_ms;    // BLINK: shortest gap between blinks
  uint16_t     blink_max_ms;    // BLINK: longest gap between blinks
};

/*
 * SpriteAnimator — draws animation frames flicker-free.
 *
 * The trick: never clear the screen. Each frame, the sprite's bounding box
 * is written ONCE, compositing transparent pixels onto the background color.
 * No blank fill between frames => no visible refresh flash.
 *
 * kind == ANIM_BLINK: holds the eyes-open frame for a random interval in
 * [blink_min_ms, blink_max_ms], flashes the eyes-closed frame for
 * blink_close_ms, then re-samples a fresh interval. Blinks never look periodic.
 *
 * Assumes all frames in one animation share w/h (true for faces).
 */
class SpriteAnimator {
public:
  SpriteAnimator(Adafruit_GC9A01A &tft, uint16_t bg = 0x0000)
      : _tft(tft), _bg(bg) {}

  void setBackground(uint16_t color) { _bg = color; }

  // Start (or restart) an animation; renders frame 0 immediately.
  void play(const Animation &a) {
    _anim       = &a;
    _idx        = 0;
    _last       = millis();
    _scale      = fitScale(a.frames[0].w, a.frames[0].h);
    _eyesClosed = false;
    if (a.kind == ANIM_BLINK)
      _nextEvent = millis() + randRange(a.blink_min_ms, a.blink_max_ms);
    render();
  }

  // Call every loop() iteration. Non-blocking.
  void update() {
    if (!_anim) return;
    if (_anim->kind == ANIM_BLINK) { updateBlink(); return; }

    // --- normal frame sequence ---
    if (millis() - _last < _anim->frame_ms) return;
    _last += _anim->frame_ms;
    if (++_idx >= _anim->count) {
      if (_anim->loop) {
        _idx = 0;
      } else {
        _idx = _anim->count - 1;  // hold final frame
        render();
        _anim = nullptr;
        return;
      }
    }
    render();
  }

  bool done() const { return _anim == nullptr; }

private:
  // Blink state machine: toggle open<->closed when _nextEvent is reached.
  void updateBlink() {
    if (_anim->count < 2) return;                       // need open + closed
    if ((int32_t)(millis() - _nextEvent) < 0) return;   // rollover-safe wait

    if (_eyesClosed) {                 // closed -> open, schedule next blink
      _eyesClosed = false;
      _idx = 0;
      render();
      _nextEvent = millis() + randRange(_anim->blink_min_ms, _anim->blink_max_ms);
    } else {                           // open -> closed, hold briefly
      _eyesClosed = true;
      _idx = 1;
      render();
      _nextEvent = millis() + _anim->blink_close_ms;
    }
  }

  static uint32_t randRange(uint32_t lo, uint32_t hi) {
    if (hi <= lo) return lo;
    return lo + (RAND32() % (hi - lo + 1));  // inclusive [lo, hi]
  }

  uint8_t fitScale(uint16_t w, uint16_t h) {
    uint8_t s = min(_tft.width() / w, _tft.height() / h);
    return s < 1 ? 1 : s;
  }

  void render() {
    const Frame  &f  = _anim->frames[_idx];
    const uint8_t s  = _scale;   // local copy: kept in a register in the loop
    const int16_t W  = f.w * s, H = f.h * s;
    const int16_t x0 = (_tft.width()  - W) / 2;
    const int16_t y0 = (_tft.height() - H) / 2;
    const uint16_t bw = (f.w + 7) / 8;

    static uint16_t line[240];  // one expanded output row (panel is 240 wide)

    _tft.startWrite();
    _tft.setAddrWindow(x0, y0, W, H);
    for (int16_t sy = 0; sy < f.h; sy++) {
      const uint16_t *srow = &f.data[sy * f.w];
      const uint8_t  *mrow = f.mask ? &f.mask[sy * bw] : nullptr;
      for (int16_t sx = 0; sx < f.w; sx++) {
        bool opaque = mrow ? (mrow[sx >> 3] & (0x80 >> (sx & 7))) : true;
        uint16_t c = opaque ? srow[sx] : _bg;
        uint16_t *dst = &line[sx * s];
        for (uint8_t k = 0; k < s; k++) dst[k] = c;  // horizontal scale
      }
      for (uint8_t r = 0; r < s; r++)                // vertical scale
        _tft.writePixels(line, W);
    }
    _tft.endWrite();
  }

  Adafruit_GC9A01A &_tft;
  uint16_t          _bg;
  const Animation  *_anim       = nullptr;
  uint8_t           _idx        = 0;
  uint8_t           _scale      = 1;
  uint32_t          _last       = 0;
  uint32_t          _nextEvent  = 0;      // BLINK: when to toggle next
  bool              _eyesClosed = false;  // BLINK: current eye state
};