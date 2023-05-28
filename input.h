#pragma once

struct input;
typedef struct input *input_t;

input_t in_create();
void in_free(input_t in);

void in_mouse_button_left(input_t in);
void in_mouse_button_middle(input_t in);
void in_mouse_button_right(input_t in);

void in_mouse_scroll_up(input_t in);
void in_mouse_scroll_down(input_t in);
void in_mouse_scroll_left(input_t in);
void in_mouse_scroll_right(input_t in);

void in_mouse_button_prev(input_t in);
void in_mouse_button_next(input_t in);

void in_mouse_move_to(input_t in, int x, int y);

void in_key_left(input_t in);
void in_key_right(input_t in);
void in_key_up(input_t in);
void in_key_down(input_t in);

void in_key_desk_1(input_t in);
void in_key_desk_2(input_t in);
void in_key_desk_3(input_t in);

void in_key_volume_up(input_t in);
void in_key_volume_down(input_t in);
void in_key_volume_mute(input_t in);

void in_audio_prev(input_t in);
void in_audio_next(input_t in);
void in_audio_play(input_t in);

void in_key_brightness_up(input_t in);
void in_key_brightness_down(input_t in);
void in_key_screen_off(input_t in);
