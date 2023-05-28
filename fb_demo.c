#include <stdlib.h>
#include <assert.h>
#include "fritzbox.h"

int main()
{
  fritzbox_t fb = fb_create("fritz.box", "usr", "pwd");
  assert(fb);

  int ret = fb_get_sid(fb);
  assert(ret != FB_FAILURE);

  ret = fb_get_templates(fb);
  assert(ret != FB_FAILURE);

  ret = fb_apply_template(fb, "template");
  assert(ret != FB_FAILURE);

  ret = fb_get_devices(fb);
  assert(ret != FB_FAILURE);

  int state = fb_get_state(fb, "switch");
  assert(state != FB_FAILURE);
  ret = fb_set_state(fb, "switch", !state); // toggle

  ret = fb_set_level(fb, "lamp", 100); // full
  assert(ret != FB_FAILURE);
  ret = fb_set_color(fb, "lamp", FB_WHITE_3000); // 3000K
  assert(ret != FB_FAILURE);

  fb_free(fb);
  
  return EXIT_SUCCESS;
}
