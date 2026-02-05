#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <ncurses.h>

#include "termaze.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fputs("usage: thecodinggame <level-file>\n", stderr);
    return EXIT_FAILURE;
  }
  FILE* fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fputs("error: failed opening file\n", stderr);
    return EXIT_FAILURE;
  }
  char * buffer = 0;
  size_t length;
  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buffer = malloc(sizeof(char) * length);
  if (buffer)
  {
    fread (buffer, 1, length, fp);
  }
  fclose (fp);
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  start_color();
  init_pair(BLACK, COLOR_WHITE, COLOR_BLACK);
  init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);
  init_pair(GREEN, COLOR_WHITE, COLOR_GREEN);
  init_pair(YELLOW, COLOR_WHITE, COLOR_YELLOW);
  init_pair(RED, COLOR_WHITE, COLOR_RED);
  init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
  lines_t lines = get_buff(buffer);
  parse_level(lines);

  refresh();
  for (;;) {}
  return EXIT_SUCCESS;
}

