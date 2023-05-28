#pragma once

struct image
{
  uint16_t width;
  uint16_t height;
  uint16_t pixels[1];
};
typedef struct image *image_t;

image_t img_create(int width, int height, int red, int green, int blue);

image_t img_load_ppm(const char *path);

image_t img_read(const char *path);

void img_write(image_t img, const char *path);

void img_draw_rectangle(image_t img, int x, int y, int width, int height,
  int red, int green, int blue);

void img_draw_line(image_t img, int x, int y, int xx, int yy,
  int red, int green, int blue);

void img_print_text(image_t img, int x, int y, const char *text,
  int red, int green, int blue);

void img_free(image_t img);
