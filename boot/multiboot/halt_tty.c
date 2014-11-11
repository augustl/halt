#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <halt_tty.h>

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;

static uint16_t* const terminal_buffer = (uint16_t*)0xB8000;
static const size_t vga_width = 80;
static const size_t vga_height = 25;

void halt_tty_initialize() {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = halt_tty_make_color(HALT_TTY_COLOR_LIGHT_GREY, HALT_TTY_COLOR_BLUE);

  size_t x;
  size_t y;

  for (y = 0; y < vga_height; y++) {
    for (x = 0; x < vga_width; x++) {
      const size_t index = y * vga_width + x;
      terminal_buffer[index] = halt_tty_make_vgaentry(' ', terminal_color);
    }
  }
}

void halt_tty_putchar(char c) {
  const size_t index = terminal_row * vga_width + terminal_column;
  terminal_buffer[index] = halt_tty_make_vgaentry(c, terminal_color);

  if (++terminal_column == vga_width) {
    terminal_column = 0;

    if (++terminal_row == vga_height) {
      terminal_row = 0;
    }
  }
}

void halt_tty_put(const char *str, size_t length) {
  size_t i;
  for (i = 0; i < length; i++) {
    halt_tty_putchar(str[i]);
  }

  if (++terminal_row == vga_height) {
    terminal_row = 0;
  }
  terminal_column = 0;
}

static size_t strlen(const char *string)
{
  size_t result = 0;
  while (string[result])
    result++;
  return result;
}

void halt_tty_puts(const char *str) {
  halt_tty_put(str, strlen(str));
}
