#pragma once

#include <stdint.h>

#define LD_SCREEN_WIDTH 440
#define LD_SCREEN_HEIGHT 260
#define LD_FAILURE (-1)
#define LD_TOUCH_SIZE 80
#define LD_TOUCH_STEP 90
 
typedef int loupedeck_t;

loupedeck_t ld_create(const char *pattern);
void ld_free(loupedeck_t ld);

void ld_backlight_level(loupedeck_t ld, int level);
void ld_button_color(loupedeck_t ld, int button, int red, int green, int blue);
void ld_vibrate(loupedeck_t ld, int level);

void ld_send_image(loupedeck_t ld, int x, int y, int width, int height, uint16_t *pixels);
void ld_update_screen(loupedeck_t ld);

#define LD_NO_EVENT 0
#define LD_DIAL_0_PRESS 1
#define LD_DIAL_1_PRESS 2
#define LD_BUTTON_0_PRESS 3
#define LD_BUTTON_1_PRESS 4
#define LD_BUTTON_2_PRESS 5
#define LD_BUTTON_3_PRESS 6
#define LD_DIAL_0_RELEASE 7
#define LD_DIAL_1_RELEASE 8
#define LD_BUTTON_0_RELEASE 9
#define LD_BUTTON_1_RELEASE 10
#define LD_BUTTON_2_RELEASE 11
#define LD_BUTTON_3_RELEASE 12
#define LD_DIAL_0_TURN_UP 13
#define LD_DIAL_1_TURN_UP 14
#define LD_DIAL_0_TURN_DOWN 15
#define LD_DIAL_1_TURN_DOWN 16
#define LD_TOUCH_0_0_PRESS 17
#define LD_TOUCH_1_0_PRESS 18
#define LD_TOUCH_2_0_PRESS 19
#define LD_TOUCH_3_0_PRESS 20
#define LD_TOUCH_4_0_PRESS 21
#define LD_TOUCH_0_1_PRESS 22
#define LD_TOUCH_1_1_PRESS 23
#define LD_TOUCH_2_1_PRESS 24
#define LD_TOUCH_3_1_PRESS 25
#define LD_TOUCH_4_1_PRESS 26
#define LD_TOUCH_0_2_PRESS 27
#define LD_TOUCH_1_2_PRESS 28
#define LD_TOUCH_2_2_PRESS 29
#define LD_TOUCH_3_2_PRESS 30
#define LD_TOUCH_4_2_PRESS 31
#define LD_TOUCH_0_0_RELEASE 32
#define LD_TOUCH_1_0_RELEASE 33
#define LD_TOUCH_2_0_RELEASE 34
#define LD_TOUCH_3_0_RELEASE 35
#define LD_TOUCH_4_0_RELEASE 36
#define LD_TOUCH_0_1_RELEASE 37
#define LD_TOUCH_1_1_RELEASE 38
#define LD_TOUCH_2_1_RELEASE 39
#define LD_TOUCH_3_1_RELEASE 40
#define LD_TOUCH_4_1_RELEASE 41
#define LD_TOUCH_0_2_RELEASE 42
#define LD_TOUCH_1_2_RELEASE 43
#define LD_TOUCH_2_2_RELEASE 44
#define LD_TOUCH_3_2_RELEASE 45
#define LD_TOUCH_4_2_RELEASE 46

int ld_event(loupedeck_t ld, int timeout_ms);
