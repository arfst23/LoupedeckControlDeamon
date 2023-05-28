#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <assert.h>
#include "input.h"

input_t in_create()
{
  Display *display = XOpenDisplay(NULL);
  return (input_t)display;
}

void in_free(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  XCloseDisplay(display);
}

static void mouse_click(Display *display, int button)
{
  int ret = XTestFakeButtonEvent(display, button, True, CurrentTime); // press
  assert(ret);
  ret = XFlush(display);
  assert(ret);

  usleep(12);

  ret = XTestFakeButtonEvent(display, button, False, CurrentTime); // release
  assert(ret);
  ret = XFlush(display);
  assert(ret);
}

void in_mouse_button_left(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 1);
}

void in_mouse_button_middle(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 2);
}

void in_mouse_button_right(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 3);
}

void in_mouse_scroll_up(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 4);
}

void in_mouse_scroll_down(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 5);
}

void in_mouse_scroll_left(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 6);
}

void in_mouse_scroll_right(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 7);
}

void in_mouse_button_prev(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 8);
}

void in_mouse_button_next(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  mouse_click(display, 9);
}

void in_mouse_move_to(input_t in, int x, int y)
{
  assert(in);
  Display *display = (Display*)in;
  int screen = XDefaultScreen(display);
  int width = XDisplayWidth(display, screen);
  int height = XDisplayWidth(display, screen);
  if (x < 0)
    x += width;
  if (y < 0)
    y += height;
  assert(x >= 0 && x < width);
  assert(y >= 0 && y < height);

  Window root = XDefaultRootWindow(display);
  XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
  XFlush(display);
}

static void key_click(Display *display, const char *key)
{
  // https://linux.die.net/man/3/xstringtokeysym
  KeySym keysym = XStringToKeysym(key);
  assert(keysym != NoSymbol);
  KeyCode keycode = XKeysymToKeycode(display, keysym);
  assert(keycode);

  XTestFakeKeyEvent(display, keycode, True, CurrentTime); // press
  XSync(display, False);

  usleep(12);

  XTestFakeKeyEvent(display, keycode, False, CurrentTime); // press
  XSync(display, False);
}

void in_key_left(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "Left");
}

void in_key_right(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "Right");
}

void in_key_up(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "Up");
}

void in_key_down(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "Down");
}

void in_key_desk_1(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86LaunchA");
}

void in_key_desk_2(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86LaunchB");
}

void in_key_desk_3(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86LaunchC");
}

void in_key_volume_up(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86AudioRaiseVolume");
}

void in_key_volume_down(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86AudioLowerVolume");
}

void in_key_volume_mute(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86AudioMute");
}

void in_audio_prev(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86AudioPrev");
}

void in_audio_next(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86AudioNext");
}

void in_audio_play(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86AudioPlay");
}

void in_key_brightness_up(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86MonBrightnessUp");
}

void in_key_brightness_down(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86MonBrightnessDown");
}

void in_key_screen_off(input_t in)
{
  assert(in);
  Display *display = (Display*)in;
  key_click(display, "XF86Away");
}
