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

int intlen(int n) {
  int len = 0;
  while (n/=10) {
    len++;
  }
  return len;
}

int main(int argc, char *argv[])
{
  int c;
  int tick_time_ms = 100;
  char * level_generator = NULL;
  while (true) {
    static struct option long_options[] =
    {
      {"tick-time",     required_argument,       0, 't'},
      {"level-generator",     required_argument,       0, 'g'},
      {0, 0, 0, 0}
    };
    
    c = getopt_long (argc, argv, "t:g:", long_options, 0);
    if (c == -1) {
      break;
    }
    switch (c) {
      case 0:
        break;
      case 't':
      tick_time_ms = atoi(optarg);
      break;
      case 'g':
      level_generator = optarg;
      break;
    }
  }
  if (argc - optind != 1 && (!!level_generator == (argc - optind != 0))) {
    fputs("usage: termaze [OPTION]... [FILE]\n", stderr);
    return EXIT_FAILURE;
  }
  char * level = 0;
  size_t length;
  ncurses_init();
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = handle_winch;
  sigaction(SIGWINCH, &sa, NULL);
  int maxy, maxx;
  getmaxyx(stdscr, maxy, maxx);
  if (level_generator) {
    // 3=2 spaces, one \0
    int len = strlen(level_generator) + intlen(maxy) + intlen(maxx) + 3;
    char* cmd = malloc(sizeof(char) * len);
    sprintf(cmd, "'%s' %d %d", level_generator, maxy, maxx);
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
      fputs("error: failed running generator command\n", stderr);
      return EXIT_FAILURE;
    }
    char buffer[4096];
    int size = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
      size += strlen(buffer);
      if (level == NULL) {
        level = malloc(size+1);
      } else {
        char* tmp = realloc(level, size+1);
        if (tmp) {
          level = tmp;
        }
      }
      strcat(level, buffer);
    }
    pclose(pipe);
  } else {
    FILE* fp = fopen(argv[optind++], "r");
    if (fp == NULL) {
      fputs("error: failed opening file\n", stderr);
      return EXIT_FAILURE;
    }
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    level = malloc(sizeof(char) * length);
    if (level)
    {
      fread(level, 1, length, fp);
    }
    fclose(fp);
  }
  #if __DEBUG
  fputs("game init started\n", stderr);
  #endif // __DEBUG
  game = game_init(level);
  #if __DEBUG
  fputs("game init successful\n", stderr);
  #endif // __DEBUG
  wrefresh(game.win);

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
    wrefresh(game.win);
  }
  for (;;) {
    sleep(UINT_MAX);
  }
  return EXIT_SUCCESS;
}

