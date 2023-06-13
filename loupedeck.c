#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "loupedeck.h"

#define DEV_SERIAL "/dev/serial/by-id"
#define LOUPEDECK "Loupedeck_Live_S"

static const char *ld_find_device(const char *pattern)
{
  DIR *dir = opendir(DEV_SERIAL);
  if (!dir)
    return NULL;

  int matches = 0;
  static char device[18 + 63 + 20 + 1];
  for (struct dirent *ent; (ent = readdir(dir)); )
  {
    if (!strstr(ent->d_name, LOUPEDECK))
      continue;

    if (pattern && !strstr(ent->d_name, pattern))
      continue;
      
    strcpy(device, DEV_SERIAL);
    strcat(device, "/");
    strcat(device, ent->d_name);
    matches++;
  }

  closedir(dir);

  if (matches != 1)
    return NULL;
  return device;
}  

static int ld_open_device(const char *device)
{
  assert(device);

  int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0)
    return LD_FAILURE;

  struct termios options;
  int ret = tcgetattr(fd, &options);
  assert(!ret);

  // input mode flags
  options.c_iflag = 0; // clear all
  
  // output mode flags
  options.c_oflag &= ~OPOST;

  // control mode flags
  options.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD | CRTSCTS);
  options.c_cflag |= CS8 | CREAD | CLOCAL;

  // local mode flag
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN);

  // line discipline
  options.c_line = 0; // clear all

  // control characters
  options.c_cc[VMIN] = 0;
  options.c_cc[VTIME] = 0;

  cfsetispeed(&options, B1152000); // input speed
  cfsetospeed(&options, B1152000); // output speed

  ret = tcsetattr(fd, TCSANOW, &options);
  assert(!ret);

  return fd;
}

static void ld_send(int fd, const void *data, int length)
{
  while (length > 0)
  {
    fd_set set;
    FD_ZERO (&set);
    FD_SET(fd, &set);

    int ret = select(fd + 1, NULL, &set, NULL, NULL);
    assert(ret == 1);
    assert(FD_ISSET(fd, &set));

    ret = write(fd, data, length);
    assert(ret >= 0);
    data += ret;
    length -= ret;
    assert(length >= 0);
  }
}

static void ld_receive(int fd, void *data, int length)
{
  while (length)
  {
    int ret = read(fd, data, length);
    assert(ret <= length);
    length -= ret;
    data += ret;
  }
}

loupedeck_t ld_create(const char *pattern)
{
  const char *device = ld_find_device(pattern);
  if (!device)
    return LD_FAILURE;

  int fd = ld_open_device(device);
  if (fd < 0)
    return LD_FAILURE;

  // exit websocket mode in case it was already in
  ld_send(fd, "\210\200\0\0\0\0", 6);

  // enter websocket mode
  ld_send(fd, "GET /index.html HTTP/1.1\n"
    "Connection: Upgrade\n"
    "Upgrade: websocket\n"
    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n\n", 109);

  uint8_t buffer[129];
  ld_receive(fd, &buffer, 129); // dismiss 129 bytes startup
  if (buffer[0] == 0x88
    && buffer[1] == 0x02
    && buffer[2] == 0x03
    && buffer[3] == 0xe8) // restart
    ld_receive(fd, &buffer, 4); // 4 extra bytes at the beginning - catch up

  return fd;
}

void ld_free(loupedeck_t ld)
{
  close(ld);
}

void ld_backlight_level(loupedeck_t ld, int level)
{
  assert(level >= 0 && level <= 10);

  static char command[11] =
    "\x82" // command opcode
    "\x84" // 0x83 + #args
    "\0\0\0\0" // padding
    "\x04\x09\x09" // command (lsb doubled)
    "\0"; // args
  command[9] = level;

  ld_send(ld, command, 10);
}

void ld_button_color(loupedeck_t ld, int button, int red, int green, int blue)
{
  assert(button >= 0 && button <= 3);
  assert(red >= 0 && red <= 255);
  assert(green >= 0 && green <= 255);
  assert(blue >= 0 && blue <= 255);

  static char command[14] =
    "\x82" // command opcode
    "\x87" // 0x83 + #args
    "\0\0\0\0" // padding
    "\x07\x02\x02" // command (lsb doubled)
    "\0\0\0\0"; // args
  command[9] = button + 7;
  command[10] = red;
  command[11] = green;
  command[12] = blue;

  ld_send(ld, command, 13);
}

void ld_vibrate(loupedeck_t ld, int level)
{
  assert(level >= 0 && level <= 2);

  static char command[11] =
    "\x82" // command opcode
    "\x84" // 0x83 + #args
    "\0\0\0\0" // padding
    "\x04\x1b\x1b" // command (lsb doubled)
    "\0"; // args
  
  switch (level)
  {
  case 0:
    command[9] = 6;
    break;
  case 1:
    command[9] = 5;
    break;
  case 2:
    command[9] = 25;
    break;
  }

  ld_send(ld, command, 10);
}

void ld_send_image(loupedeck_t ld, int x, int y, int width, int height, uint16_t *pixels)
{
  assert(x >= 0 && x + width <= LD_SCREEN_WIDTH);
  assert(y >= 0 && y + height <= LD_SCREEN_HEIGHT);
  assert(pixels);
  x += 19;
  y += 5;

  int length = 2 * width * height;
  assert(length >= 2 && length <= 259200);
  if (length < 112) // 0x83 + 11 + length < 0xfe
  {
    static char command[21] =
      // 9
      "\x82" // command opcode
      "\0" // 0x83 + #args
      "\0\0\0\0" // padding
      "\xff\x10\x10" // command (lsb doubled)
      // 11
      "\0\x4d" // screen id
      "\0\0" // x left
      "\0\0" // y top
      "\0\0" // width
      "\0\0" // height
      "\0"; // padding
    command[1] = 0x83 + 11 + length;
    command[11] = (x >> 8) & 0xff;
    command[12] = x & 0xff;
    command[13] = (y >> 8) & 0xff;
    command[14] = y & 0xff;
    command[15] = (width >> 8) & 0xff;
    command[16] = width & 0xff;
    command[17] = (height >> 8) & 0xff;
    command[18] = height & 0xff;

    ld_send(ld, command, 20);
    ld_send(ld, pixels, length);
  }
  else if (length <= 65520) // 11 + length <= 0xffff
  {
    static char command[23] =
      // 11
      "\x82" // command opcode
      "\xfe\0\0" // 0xfe msb lsb
      "\0\0\0\0" // padding
      "\xff\x10\x10" // command (lsb doubled)
      // 11
      "\0\x4d" // screen id
      "\0\0" // x left
      "\0\0" // y top
      "\0\0" // width
      "\0\0" // height
      "\0"; // padding
    command[2] = ((length + 14) >> 8) & 0xff;
    command[3] = (length + 14) & 0xff;
    command[13] = (x >> 8) & 0xff;
    command[14] = x & 0xff;
    command[15] = (y >> 8) & 0xff;
    command[16] = y & 0xff;
    command[17] = (width >> 8) & 0xff;
    command[18] = width & 0xff;
    command[19] = (height >> 8) & 0xff;
    command[20] = height & 0xff;

    ld_send(ld, command, 22);
    ld_send(ld, pixels, length);
  }
  else
  {
    static char command[29] =
      // 17
      "\x82" // command opcode
      "\xff\0\0\0\0\0\0\0\0" // 0xff msb ... lsb
      "\0\0\0\0" // padding
      "\xff\x10\x10" // command (lsb doubled)
      // 11
      "\0\x4d" // screen id
      "\0\0" // x left
      "\0\0" // y top
      "\0\0" // width
      "\0\0" // height
      "\0"; // padding
    command[7] = ((length + 14) >> 16) & 0xff;
    command[8] = ((length + 14) >> 8) & 0xff;
    command[9] = (length + 14) & 0xff;
    command[19] = (x >> 8) & 0xff;
    command[20] = x & 0xff;
    command[21] = (y >> 8) & 0xff;
    command[22] = y & 0xff;
    command[23] = (width >> 8) & 0xff;
    command[24] = width & 0xff;
    command[25] = (height >> 8) & 0xff;
    command[26] = height & 0xff;

    ld_send(ld, command, 28);
    ld_send(ld, pixels, length);
  }
}

void ld_update_screen(loupedeck_t ld)
{
  static char command[12] =
    "\x82" // command opcode
    "\x85" // 0x83 + #args
    "\0\0\0\0" // padding
    "\x05\x0f\x0f" // command (lsb doubled)
    "\0\x4d"; // args

  ld_send(ld, command, 11);
}

int ld_event(loupedeck_t ld, int timeout_ms)
{
  while (1)
  {
    fd_set set;
    FD_ZERO (&set);
    FD_SET(ld, &set);

    if (timeout_ms < 0) // no timeout
    {
      int ret = select(ld + 1, &set, NULL, NULL, NULL);
      assert(ret == 1);
    }
    else
    {
      struct timeval timeout = { timeout_ms / 1000, timeout_ms % 1000 * 1000 };
      if (!timeout_ms)
	timeout.tv_usec = 1;
      int ret = select(ld + 1, &set, NULL, NULL, &timeout);
      if (!ret)
	return LD_NO_EVENT;
      assert(ret == 1);
    }
    assert(FD_ISSET(ld, &set));

    uint8_t data[6];
    ld_receive(ld, &data, 5);
    if (data[0] == 0x82
      && data[1] == 0x05
      && data[2] == 0x05
      && data[3] == 0
      && data[4] == 0)
    {
      ld_receive(ld, &data, 2);
      if (!data[1]) // press
	if (data[0] <= 2) // dial
	  return LD_DIAL_0_PRESS + data[0] - 1;
	else // button
	  return LD_BUTTON_0_PRESS + data[0] - 7;
      else // release
	if (data[0] <= 2) // dial
	  return LD_DIAL_0_RELEASE + data[0] - 1;
	else // button
	  return LD_BUTTON_0_RELEASE + data[0] - 7;
    }
    else if (data[0] == 0x82
      && data[1] == 0x05
      && data[2] == 0x05
      && data[3] == 1
      && data[4] == 0)
    {
      ld_receive(ld, &data, 2);
      if (data[1] == 1) // up
	return LD_DIAL_0_TURN_UP + data[0] - 1;
      else // down
	return LD_DIAL_0_TURN_DOWN + data[0] - 1;
    }
    else if (data[0] == 0x82
      && data[1] == 0x09
      && data[2] == 0x09
      && data[4] == 0)
    {
      int press = data[3] == 0x4d;
      ld_receive(ld, &data, 6);
      int x = 256 * data[1] + data[2];
      int xx = (x - 30) / 90;
      int xxx = (x + 9) / 90;
      if (xx != xxx)
	continue;
      int y = 256 * data[3] + data[4];
      int yy = (y - 30) / 90;
      int yyy = (y + 9) / 90;
      if (yy != yyy)
	continue;
      
      if (press)
	return LD_TOUCH_0_0_PRESS + xx + 5 * yy;
      else
	return LD_TOUCH_0_0_RELEASE + xx + 5 * yy;
    }
    else if (data[0] == 0x82
      && data[1] == 0x04
      && data[2] == 0x04)
    {
      // ack backlight level / vibrate / send image
      ld_receive(ld, &data, 1);
    }
  }
}
