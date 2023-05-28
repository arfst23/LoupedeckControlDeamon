#pragma once

#define FB_RED 0
#define FB_ORANGE 1
#define FB_YELLOW 2
#define FB_CHARTREUSE 3
#define FB_GREEN 4
#define FB_TURQUOISE 5
#define FB_CYAN 6
#define FB_AZURE 7
#define FB_BLUE 8
#define FB_PURPLE 9
#define FB_MAGENTA 10
#define FB_PINK 11
#define FB_WHITE_2700 12
#define FB_WHITE_3000 13
#define FB_WHITE_3400 14
#define FB_WHITE_3800 15
#define FB_WHITE_4200 16
#define FB_WHITE_4700 17
#define FB_WHITE_5300 18
#define FB_WHITE_5900 19
#define FB_WHITE_6500 20

#define FB_SUCCESS 0
#define FB_FAILURE (-1)

struct fritzbox;
typedef struct fritzbox *fritzbox_t;

fritzbox_t fb_create(const char *hostname, const char *user, const char *password);
int fb_get_sid(fritzbox_t fb);
void fb_free(fritzbox_t fb);

int fb_get_templates(fritzbox_t fb);
int fb_apply_template(fritzbox_t fb, const char *template_name);

int fb_get_devices(fritzbox_t fb);
int fb_get_state(fritzbox_t fb, const char *device_name);
int fb_set_state(fritzbox_t fb, const char *device_name, int state);
int fb_get_level(fritzbox_t fb, const char *device_name);
int fb_set_level(fritzbox_t fb, const char *device_name, int level);
int fb_get_color(fritzbox_t fb, const char *device_name);
int fb_set_color(fritzbox_t fb, const char *device_name, int color);
