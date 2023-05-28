#include <stdlib.h>
#include <assert.h>
#include "loupedeck.h"
#include "image.h"

#include <stdio.h>

int main()
{
  loupedeck_t ld = ld_create(NULL);
  assert(ld >= 0);

  ld_backlight_level(ld, 3);
  ld_button_color(ld, 1, 128, 0, 0);

  image_t img = img_create(LD_TOUCH_SIZE, LD_TOUCH_SIZE, 200, 0, 200);
  img_print_text(img, 10, 10, "Text", 0, 0, 0);
  ld_send_image(ld, 0, 0, img->width, img->height, img->pixels);
  img_free(img);
  ld_update_screen(ld);

  ld_vibrate(ld, 1);
  ld_button_color(ld, 0, 200, 0, 200);
  ld_button_color(ld, 1, 0, 0, 0);
  ld_button_color(ld, 2, 0, 0, 0);
  ld_button_color(ld, 3, 0, 0, 0);

  while (1)
  {
    int ret = ld_event(ld, -1);
    if (ret >= LD_DIAL_0_PRESS && ret <= LD_DIAL_1_PRESS)
      printf("dial %d press\n", ret - LD_DIAL_0_PRESS);
    else if (ret >= LD_BUTTON_0_PRESS && ret <= LD_BUTTON_3_PRESS)
      printf("button %d press\n", ret - LD_BUTTON_0_PRESS);
    else if (ret >= LD_DIAL_0_RELEASE && ret <= LD_DIAL_1_RELEASE)
      printf("dial %d release\n", ret - LD_DIAL_0_RELEASE);
    else if (ret >= LD_BUTTON_0_RELEASE && ret <= LD_BUTTON_3_RELEASE)
      printf("button %d release\n", ret - LD_BUTTON_0_RELEASE);
    else if (ret >= LD_DIAL_0_TURN_UP && ret <= LD_DIAL_1_TURN_UP)
      printf("dial %d turn up\n", ret - LD_DIAL_0_TURN_UP);
    else if (ret >= LD_DIAL_0_TURN_DOWN && ret <= LD_DIAL_1_TURN_DOWN)
      printf("dial %d turn down\n", ret - LD_DIAL_0_TURN_DOWN);
    else if (ret >= LD_TOUCH_0_0_PRESS && ret <= LD_TOUCH_4_2_PRESS)
    {
      int x = (ret - LD_TOUCH_0_0_PRESS) % 5;
      int y = (ret - LD_TOUCH_0_0_PRESS) / 5;
      printf("touch %d %d press\n", x, y);
    }
    else if (ret >= LD_TOUCH_0_0_RELEASE && ret <= LD_TOUCH_4_2_RELEASE)
    {
      int x = (ret - LD_TOUCH_0_0_RELEASE) % 5;
      int y = (ret - LD_TOUCH_0_0_RELEASE) / 5;
      printf("touch %d %d release\n", x, y);
    }
    else
      assert(0);
    if (ret == LD_BUTTON_0_PRESS)
      break;
  }

  ld_button_color(ld, 0, 0, 0, 0);
  ld_free(ld);

  return EXIT_SUCCESS;
}
