#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

typedef struct {
  float x;
  float y;
} vec2d;

#define MAX_BODY_SIZE 100
typedef struct {
  vec2d body[MAX_BODY_SIZE];
  int bodySize;
  float angle;
} game;

static int update(void *userdata);
const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont *font = NULL;
game state = {0};

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t arg) {
  (void)arg; // arg is currently only used for event = kEventKeyPressed

  if (event == kEventInit) {
    const char *err;
    font = pd->graphics->loadFont(fontpath, &err);

    if (font == NULL)
      pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__,
                        fontpath, err);

    // Note: If you set an update callback in the kEventInit handler, the system
    // assumes the game is pure C and doesn't run any Lua code in the game
    pd->system->setUpdateCallback(update, pd);

    state.angle = 0;
    state.bodySize = 25;
    for (int i = 0; i < state.bodySize; i++) {
      state.body[i].x = 50;
      state.body[i].y = 50;
    }
  }

  return 0;
}

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

int x = (400 - TEXT_WIDTH) / 2;
int y = (240 - TEXT_HEIGHT) / 2;

static int update(void *userdata) {
  PlaydateAPI *pd = userdata;

  pd->graphics->clear(kColorWhite);
  // pd->graphics->setFont(font);
  // pd->graphics->drawText("new text!!", strlen("Hello World!"),
  // kASCIIEncoding, x, y);

  // angle = the lever angle
  const float angle = pd->system->getCrankAngle() * 3.14159f / 180.0f;
  const float dx = cosf(angle);
  const float dy = sinf(angle);

  PDButtons current;
  pd->system->getButtonState(&current, NULL, NULL);
  const float speed = current & kButtonA ? 2.0f : 1.0f;
  // move the head
  state.body[0].x += dx * speed;
  state.body[0].y += dy * speed;

  const int radius = 5;
  for (int i = 1; i < state.bodySize; i++) {
    vec2d prev = state.body[i - 1];
    vec2d cur = state.body[i];
    const float dist = sqrtf((cur.x - prev.x) * (cur.x - prev.x) +
                             (cur.y - prev.y) * (cur.y - prev.y));
    if (dist > 2 * radius) {
      const float angle = atan2f(cur.y - prev.y, cur.x - prev.x);
      state.body[i].x = prev.x + cosf(angle) * 2 * radius;
      state.body[i].y = prev.y + sinf(angle) * 2 * radius;
    }
  }

  // draw in reverse order
  for (int i = state.bodySize - 1; i >= 0; i--) {
    pd->graphics->fillEllipse(state.body[i].x - radius,
                              state.body[i].y - radius, radius * 2, radius * 2,
                              0, 360, kColorBlack);
    // pd->graphics->drawEllipse(state.body[i].x - radius,
    //                           state.body[i].y - radius, radius * 2, radius *
    //                           2, 1, 0, 360, kColorBlack);
    const int innerRadius = radius - 1;
    pd->graphics->fillEllipse(state.body[i].x - innerRadius,
                              state.body[i].y - innerRadius, innerRadius * 2,
                              innerRadius * 2, 0, 360, kColorWhite);
  }

  pd->system->drawFPS(0, 0);

  return 1;
}
