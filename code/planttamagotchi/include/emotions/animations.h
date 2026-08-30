#pragma once
#include "sprite_player.h"

#include "hangry_0.h"
#include "happy_0.h"
#include "happy_1.h"
#include "neutral_0.h"
#include "neutral_1.h"
#include "thirsty_0.h"
#include "thirsty_1.h"

static const Frame hangry_frames[] = {
  { hangry_0_data, hangry_0_mask, HANGRY_0_W, HANGRY_0_H },  // static (add a _1 closed frame to blink)
};
const Animation ANIM_HANGRY = { hangry_frames, 1, 150, false };

static const Frame happy_frames[] = {
  { happy_0_data, happy_0_mask, HAPPY_0_W, HAPPY_0_H },  // eyes open
  { happy_1_data, happy_1_mask, HAPPY_1_W, HAPPY_1_H },  // eyes closed
};
const Animation ANIM_HAPPY = {
  happy_frames, 2, 0, true,
  ANIM_BLINK, 120, 2500, 6000
};

static const Frame neutral_frames[] = {
  { neutral_0_data, neutral_0_mask, NEUTRAL_0_W, NEUTRAL_0_H },  // eyes open
  { neutral_1_data, neutral_1_mask, NEUTRAL_1_W, NEUTRAL_1_H },  // eyes closed
};
const Animation ANIM_NEUTRAL = {
  neutral_frames, 2, 0, true,
  ANIM_BLINK, 120, 2500, 6000
};

static const Frame thirsty_frames[] = {
  { thirsty_0_data, thirsty_0_mask, THIRSTY_0_W, THIRSTY_0_H },  // eyes open
  { thirsty_1_data, thirsty_1_mask, THIRSTY_1_W, THIRSTY_1_H },  // eyes closed
};
const Animation ANIM_THIRSTY = {
  thirsty_frames, 2, 0, true,
  ANIM_BLINK, 120, 2500, 6000
};

// Table of every animation, for menus / iteration:
struct NamedAnim { const char *name; const Animation *anim; };
static const NamedAnim ALL_ANIMS[] = {
  { "hangry", &ANIM_HANGRY },
  { "happy", &ANIM_HAPPY },
  { "neutral", &ANIM_NEUTRAL },
  { "thirsty", &ANIM_THIRSTY },
};