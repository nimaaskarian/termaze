#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
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
    fread(buffer, 1, length, fp);
  }
  fclose(fp);
  #if __DEBUG
  fputs("game init started\n", stderr);
  #endif // __DEBUG
  game_t game = game_init(buffer);
  #if __DEBUG
  fputs("game init successful\n", stderr);
  #endif // __DEBUG
  refresh();

  char line[256];
  struct timespec ts;
  static const int milliseconds = 200;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  while (fgets(line, sizeof(line), stdin)) {
    nanosleep(&ts, NULL);
    if (strcmp(line, "move\n") == 0) {
      game_move_player(&game);
    } else if (strcmp(line, "rotate\n") == 0) {
      game_rotate_player(&game, 1);
    } else if (strcmp(line, "rotate -90\n") == 0) {
      game_rotate_player(&game, -1);
    } else if (strcmp(line, "light\n") == 0) {
      game_light(&game);
    }
    #if __DEBUG
    fprintf(stderr, "info: cmd got \"%s\"\n", line);
    #endif // __DEBUG
    refresh();
  }
  for (;;) {
    sleep(UINT_MAX);
  }
  return EXIT_SUCCESS;
}

