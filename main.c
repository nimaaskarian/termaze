#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <ncurses.h>

#include "termaze.h"
game_t game;

void handle_winch(int sig)
{
    endwin();
    // Needs to be called after an endwin() so ncurses will initialize
    // itself with the new terminal dimensions.
    refresh();
    clear();
    game_redraw(&game);
    refresh();
}

int main(int argc, char *argv[])
{
  int c;
  int tick_time_ms = 100;
  while (true) {
    static struct option long_options[] =
          {
            {"tick-time",     required_argument,       0, 't'},
            {0, 0, 0, 0}
          };
    
    c = getopt_long (argc, argv, "t", long_options, 0);
    if (c == -1) {
      break;
    }
    switch (c) {
      case 0:
        break;
      case 't':
      tick_time_ms = atoi(optarg);
    }
  }
  if (argc-optind != 1) {
    fputs("usage: termaze [OPTION]... [FILE]\n", stderr);
    return EXIT_FAILURE;
  }
  FILE* fp = fopen(argv[optind++], "r");
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
  game = game_init(buffer);
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = handle_winch;
  sigaction(SIGWINCH, &sa, NULL);
  #if __DEBUG
  fputs("game init successful\n", stderr);
  #endif // __DEBUG
  refresh();

  char line[256];
  while (fgets(line, sizeof(line), stdin)) {
    napms(tick_time_ms);
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

