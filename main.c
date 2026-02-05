#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <ncurses.h>


int parse_times(const char ** input);
void parse_level(const char * input);

enum Color {
  WHITE = 1,
  GREEN,
  YELLOW,
  RED,
  BLUE,
};

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
  init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
  init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(RED, COLOR_RED, COLOR_BLACK);
  init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
  parse_level(buffer);
  refresh();
  for (;;) {}
  return EXIT_SUCCESS;
}

void parse_level(const char * input) {
  int x = 0, y = 0;
  bool found_start = false;
  for (;*input; input++) {
    switch (*input) {
      case 'P':
        attron(COLOR_PAIR(GREEN));
        mvprintw(y, x, "█");
        x+=1;
      break;
      case 'L':
        attron(COLOR_PAIR(YELLOW));
        mvprintw(y, x, "█");
        x+=1;
      break;
      case 'S':
        attron(COLOR_PAIR(RED));
        mvprintw(y, x, "█");
        x+=1;
        found_start = 1;
      break;
      case ' ':
        x+=1;
      break;
      case '\n':
        y+=1;
        x=0;
      break;
    }
  }
  if (!found_start) {
    attron(COLOR_PAIR(RED));
    mvprintw(0, 0, "█");
  }
}
