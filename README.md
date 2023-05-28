# Loupedeck Live S

```
loupedeck.h
loupedeck.c
```

Simple interface to access a Loupedeck Live S device.

```
// loupedeck_t ld_create(const char *pattern);
loupedeck_t ld = ld_create(NULL);
assert(ld != LD_FAILURE)
```

Connect to a Loupedeck Live S device.  If multiple devices are connected a
unique substring of the serial number can be handed to `ld_create`.

```
// void ld_backlight_level(loupedeck_t ld, int level);
ld_backlight_level(ld, 50);
```

Set the backlight level from 0 = off to 100 = brightest

```
// void ld_button_color(loupedeck_t ld, int button, int red, int green, int blue);
ld_button_color(ld, 0, 255, 0, 0);
```

Set the color of one of the hardware buttons.  Buttons count from 0 = left,
1 = right top to 3 = right bottom

The color is specified by its rgb values ranging from 0 to 255.

```
// void ld_vibrate(loupedeck_t ld, int level);
ld_vibrate(ld, 1);
```

Vibrate the device, give the intensity level from 0 = low to 2 = high.

```
#define LD_SCREEN_WIDTH 440
#define LD_SCREEN_HEIGHT 260
#define LD_TOUCH_SIZE 80
#define LD_TOUCH_STEP 90
```

The usable screen size is 440x260.
The touchable areas are 80x80 pixel squares, offset by 90 pixels.

```
// void ld_send_image(loupedeck_t ld, int x, int y, int width, int height, uint16_t *pixels);
ld_send_image(ld, 0, 0, image->width, image->height, image->pixels);

// void ld_update_screen(loupedeck_t ld);
void ld_update_screen(ld);
```

Send an image to (a part) of the screen, handing the coordinates of the upper
left corner, the size of the image, and the pixel values.

The update(s) become visible on the device when calling `ld_update_screen`.

```
// int ld_event(loupedeck_t ld, int timeout_ms);
int ev = ld_event(ld, -1);
switch (ev)
{
case LD_DIAL_0_TURN_UP:
  ...
  break;
case LD_TOUCH_3_1_RELEASE:
  ...
  break;
}
```

Wait for and receive events from the device.  A timeout of -1 will wait until an
event is received.  Otherwise wait for at most `timeout_ms` milliseconds.  If
the timeout is reached and no event happened `LD_NO_EVENT` is returned;

Events are
* turn dial up/down
* press/release dials or hardware buttons
* press/release touchable fields

Moving your finger on the display will result in multiple touch press events in
a row followed by a touch release event when lifting the finger.

```
// void ld_free(loupedeck_t ld);
ld_free(ld);
```

Close the connection to the device;

## Image encoding for the Loupedeck Live S device

```
image.h
image.c
```

Image handling and basic drawing primitives for images that can be send to a
Loupedeck Live S device.

```
// image_t image_create(int width, int height, int red, int green, int blue);
image_t im = image_create(80, 80, 0, 0, 255);
```

Create an image of the given size with a color specified by its rgb values
ranging from 0 to 255.

```
// image_t image_load_ppm(const char *path);
image_t im = image_load_ppm("image.ppm");
```

Load a portable pixmap image from disk.

```
// void image_draw_rectangle(image_t image, int x, int y,
//   int width, int height, int red, int green, int blue);
```

Draw a filled rectangle, giving the coordinates of its upper left corner, the
size of the image, and its color specified by its rgb values ranging from 0 to
255.

```
// void image_draw_line(image_t image, int x, int y,
//   int xx, int yy, int red, int green, int blue);
```

Draw a line from `x, y` to `xx, yy` in the color specified by its rgb values
ranging from 0 to 255.

```
// void image_print_text(image_t image, int x, int y,
//   const char *text, int red, int green, int blue);
```

Print text on the image with a monospace 8x14 pixel font, only ASCII characters
are defined.  Provide the upper left corner, the text to show, and its color
specified by its rgb values ranging from 0 to 255.

```
// void image_free(image_t image);
image_free(im);
```

Free the memory allocated for the image.

# Fritz!Box Smart Home

```
fritzbox.h
fritzbox.c
```

Simple interface to access the smart home devices connected to a Fritz!Box.

```
// fritzbox_t fb_create(const char *hostname, const char *user, const char *password);
fritzbox_t fb = fb_create("fritz.box", "usr", "pwd");
assert(fb);
```

Allocate resources to connect to a Fritz!Box and control smart home devices.

```
// int fb_get_sid(fritzbox_t fb);
int ret = fb_get_sid(fb);
assert(ret != FB_FAILURE);
```

Get/update a session id.  A valid session id is necessary to perform operations
on smart home devices.

```
// int fb_get_templates(fritzbox_t fb);
int ret = fb_get_templates(fb);
assert(ret != FB_FAILURE);
```

(Re-) Load an inventory of the templates defined in the Fritz!Box router.  This
inventory needs to be loaded before executing a template.

```
// int fb_apply_template(fritzbox_t fb, const char *template_name);
int ret = fb_apply_template(fb, "template");
assert(ret != FB_FAILURE);
```

Execute the template with the given template name.

``` 
// int fb_get_devices(fritzbox_t fb);
int ret = fb_get_devices(fb);
assert(ret != FB_FAILURE);
```

(Re-) Load an inventory of all connected switchable devices (switches and
lamps).  This inventory needs to be loaded before getting or setting device
attributes.

```
// int fb_get_state(fritzbox_t fb, const char *device_name);
int state = fb_get_state(fb, "device");
assert(state != FB_FAILURE);
```

Get the (cached) state of the device specified by its name (0 = off, 1 = on).

```
// int fb_set_state(fritzbox_t fb, const char *device_name, int state);
int ret = fb_set_state(fb, "device", 1);
assert(ret != FB_FAILURE);
```

Set the state of the device specified by its name (0 = off, 1 = on).

```
// int fb_get_level(fritzbox_t fb, const char *device_name);
int level = fb_get_level(fb, "lamp");
assert(level != FB_FAILURE);
// int fb_set_level(fritzbox_t fb, const char *device_name, int level);
int ret = fb_get_level(fb, "lamp", 50);
assert(ret != FB_FAILURE);
```

Get the (cached) or set the brightness level in percent (0 to 100) of a lamp
device specified by its name.

```
// int fb_get_color(fritzbox_t fb, const char *device_name);
int color = fb_get_color(fb, "lamp");
assert(color != FB_FAILURE);
// int fb_set_color(fritzbox_t fb, const char *device_name, int color);
int ret = fb_set_color(fb, "lamp", FB_MAGENTA);
assert(ret != FB_FAILURE);
```

Get the (cached) or set the color or white color temperature of a lamp device
specified by its name.

```
// void fb_free(fritzbox_t fb);
fb_free(fb);
```

Release all resources.
