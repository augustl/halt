#ifndef _HALT_TTY_H
#define _HALT_TTY_H

#include <stdint.h>

enum halt_tty_vga_color {
  HALT_TTY_COLOR_BLACK = 0,
  HALT_TTY_COLOR_BLUE = 1,
  HALT_TTY_COLOR_GREEN = 2,
  HALT_TTY_COLOR_CYAN = 3,
  HALT_TTY_COLOR_RED = 4,
  HALT_TTY_COLOR_MAGENTA = 5,
  HALT_TTY_COLOR_BROWN = 6,
  HALT_TTY_COLOR_LIGHT_GREY = 7,
  HALT_TTY_COLOR_DARK_GREY = 8,
  HALT_TTY_COLOR_LIGHT_BLUE = 9,
  HALT_TTY_COLOR_LIGHT_GREEN = 10,
  HALT_TTY_COLOR_LIGHT_CYAN = 11,
  HALT_TTY_COLOR_LIGHT_RED = 12,
  HALT_TTY_COLOR_LIGHT_MAGENTA = 13,
  HALT_TTY_COLOR_LIGHT_BROWN = 14,
  HALT_TTY_COLOR_WHITE = 15,
};

static inline uint8_t halt_tty_make_color(enum halt_tty_vga_color fg, enum halt_tty_vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t halt_tty_make_vgaentry(char c, uint8_t color) {
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

void halt_tty_initialize();
void halt_tty_put(const char *str, size_t length);
void halt_tty_puts(const char *str);
void halt_tty_putchar(char c);

#endif
