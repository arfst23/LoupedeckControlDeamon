#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "loupedeck.h"
#include "image.h"

#include <stdio.h>

#define PADDLE_WIDTH 5
#define PADDLE_HEIGHT 40
#define PADDLE_STEP 5
#define CENTER_X 238
#define CENTER_Y 135
#define BALL 5

int main()
{
  srand(time(NULL));

  // DarkGoldenrod1
  int red = 255;
  int green = 185;
  int blue = 15;

  loupedeck_t ld = ld_create(NULL);
  assert(ld >= 0);

  ld_backlight_level(ld, 3);
  
  ld_button_color(ld, 0, red, green, blue);
  image_t splash = img_create(480, 270, 0, 0, 0);

  img_print_text(splash, 215, 40, "PONG!", red, green, blue);
  img_print_text(splash, 25, 25, "< left", red, green, blue);
  img_print_text(splash, 25, 140, "< right", red, green, blue);
  img_print_text(splash, 25, 250, "< start", red, green, blue);
  img_print_text(splash, 400, 250, "exit >", red, green, blue);
  ld_send_image(ld, 0, 0, splash->width, splash->height, splash->pixels);
  img_free(splash);
  ld_update_screen(ld);
 
  while(1)
  {
    int event = ld_event(ld, -1);
    if (event == LD_BUTTON_0_RELEASE)
      break;
    if (event == LD_BUTTON_3_RELEASE)
      goto EXIT;
  }

  image_t black = img_create(480, 270, 0, 0, 0);

  image_t paddle = img_create(PADDLE_WIDTH, PADDLE_HEIGHT + 2 * PADDLE_STEP, 0, 0, 0);
  img_draw_rectangle(paddle, 0, PADDLE_STEP, PADDLE_WIDTH, PADDLE_HEIGHT, red, green, blue);

  image_t ball = img_create(3 * BALL, 3 * BALL, 0, 0, 0);
  img_draw_rectangle(ball, BALL, BALL, BALL, BALL, red, green, blue);

  int left_n = 0;
  int right_n = 0;

  while (1)
  {
    ld_send_image(ld, 0, 0, black->width, black->height, black->pixels);

    int left_y = CENTER_Y - PADDLE_HEIGHT / 2;
    int right_y = CENTER_Y - PADDLE_HEIGHT / 2;
    float ball_x = CENTER_X - BALL / 2;
    float ball_y = CENTER_Y - BALL / 2;

    float left_v = 0.0;
    float right_v = 0.0;

    ld_send_image(ld, (int)ball_x - BALL, (int)ball_y - BALL,
      ball->width, ball->height, ball->pixels);
    ld_send_image(ld, 19 + 5, left_y - PADDLE_STEP,
      paddle->width, paddle->height, paddle->pixels);
    ld_send_image(ld, 458 - 5 - PADDLE_WIDTH, right_y - PADDLE_STEP,
      paddle->width, paddle->height, paddle->pixels);
    ld_update_screen(ld);

    while(1)
    {
      int event = ld_event(ld, -1);
      if (event == LD_BUTTON_0_RELEASE)
	break;
      if (event == LD_BUTTON_3_RELEASE)
	goto END;
    }
    ld_button_color(ld, 0, 0, 0, 0);

    float ball_dx = 0.05;
    if (rand() & 1)
      ball_dx = -ball_dx;

    float ball_dy = (rand() % 21 - 10) * 0.01;

    while (1)
    {
      int event;
      int left_d = 0;
      int right_d = 0;

      while ((event = ld_event(ld, 0)))
	switch (event)
	{
	case LD_DIAL_0_TURN_UP:
	  left_d = 1;
	  break;
	case LD_DIAL_0_TURN_DOWN:
	  left_d = -1;
	  break;
	case LD_DIAL_1_TURN_UP:
	  right_d = 1;
	  break;
	case LD_DIAL_1_TURN_DOWN:
	  right_d = -1;
	  break;
	case LD_BUTTON_3_RELEASE:
	  goto END;
	}
      if (left_d < 0 && left_y > 5)
	left_y -= PADDLE_STEP;
      if (left_d > 0 && left_y < 264 - PADDLE_HEIGHT)
	left_y += PADDLE_STEP;
      if (right_d < 0 && right_y > 5)
	right_y -= PADDLE_STEP;
      if (right_d > 0 && right_y < 264 - PADDLE_HEIGHT)
	right_y += PADDLE_STEP;

      left_v *= 0.97;
      left_v += left_d;
      right_v *= 0.97;
      right_v += right_d;

      ball_x += ball_dx;
      ball_y += ball_dy;

      if (ball_x + BALL > 458 - 5 - PADDLE_WIDTH)
      {
	if (ball_y >= right_y - 2 && ball_y + BALL <= right_y + PADDLE_HEIGHT + 2)
	{
	  ball_dx = -ball_dx;
	  ball_x += ball_dx;
	  ball_dy += right_v * 0.1;
	}
	else
	{
	  ld_vibrate(ld, 2);
	  left_n++;
	  goto FIN;
	}
      }

      if (ball_x < 19 + 5 + PADDLE_WIDTH)
      {
	if (ball_y >= left_y - 2 && ball_y + BALL <= left_y + PADDLE_HEIGHT + 2)
	{
	  ball_dx = -ball_dx;
	  ball_x += ball_dx;
	  ball_dy += left_v * 0.1;
	}
	else
	{
	  ld_vibrate(ld, 2);
	  right_n++;
	  goto FIN;
	}
      }

      if (ball_y < 5)
      {
	ball_dy = -ball_dy;
	ball_y += ball_dy;
	ball_dy *= 0.9;
      }
      if (ball_y + BALL > 264)
      {
	ball_dy = -ball_dy;
	ball_y += ball_dy;
	ball_dy *= 0.9;
      }

      ld_send_image(ld, ball_x - BALL, ball_y - BALL,
	ball->width, ball->height, ball->pixels);
      ld_send_image(ld, 19 + 5, left_y - PADDLE_STEP,
	paddle->width, paddle->height, paddle->pixels);
      ld_send_image(ld, 458 - 5 - PADDLE_WIDTH, right_y - PADDLE_STEP,
	paddle->width, paddle->height, paddle->pixels);
      ld_update_screen(ld);
    }

  FIN:
    image_t score = img_create(80, 20, 0, 0, 0);
    char score_str[24]; // [8] if fine but gcc issues warnings
    sprintf(score_str, "%02d : %02d", left_n, right_n);
    img_print_text(score, 10, 5, score_str, red, green, blue);
    ld_send_image(ld, 199, 110, score->width, score->height, score->pixels);
    img_free(score);
    ld_update_screen(ld);

    ld_button_color(ld, 0, red, green, blue);

    while(1)
    {
      int event = ld_event(ld, -1);
      if (event == LD_BUTTON_0_RELEASE)
	break;
      if (event == LD_BUTTON_3_RELEASE)
	goto END;
    }
  }

END:
  ld_send_image(ld, 0, 0, black->width, black->height, black->pixels);
  ld_update_screen(ld);

  img_free(ball);
  img_free(paddle);
  img_free(black);

EXIT:
  ld_button_color(ld, 0, 0, 0, 0);
  ld_free(ld);

  return EXIT_SUCCESS;
}
