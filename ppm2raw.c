#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "image.h"

int main(int ac, char *av[])
{
  if (ac != 3)
  {
    fprintf(stderr, "usage: %s in.ppm out.raw\n", *av);
    return EXIT_FAILURE;
  }

  image_t img = img_load_ppm(av[1]);
  img_write(img, av[2]);

  return EXIT_SUCCESS;
}
