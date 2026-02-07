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
#include "util.h"

typedef struct app {
  int tick_time_ms;
  char * level_generator;

  game_t game;
  int maxx, maxy;
  WINDOW* win;
  char* msg;
  char* level;
} app_t;

void app_init_level(app_t* app, char** argv, int argc);
void app_play_line(app_t* app, char* line, char**prevmsg, int* failed_sum);
void handle_winch(int sig);
int intlen(int n);

app_t app;

int main(int argc, char *argv[])
{
  int c;
  app.tick_time_ms = 100;
  app.level_generator = NULL;
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
      app.tick_time_ms = atoi(optarg);
      break;
      case 'g':
      app.level_generator = optarg;
      break;
    }
  }
  if (argc - optind != 1 && (!!app.level_generator == (argc - optind != 0))) {
    die("usage: termaze [OPTION]... [FILE]");
  }
  ncurses_init();
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = handle_winch;
  sigaction(SIGWINCH, &sa, NULL);

  // init app
  getmaxyx(stdscr, app.maxy, app.maxx);
  app_init_level(&app, argv, argc);
  app.game = game_init(app.level);
  app.win = newwin(2, app.maxx, app.maxy-2, 0);

  char line[256];
  int failed_sum = 0;
  char * prevmsg = NULL;
  while (fgets(line, sizeof(line), stdin)) {
    app_play_line(&app, line, &prevmsg, &failed_sum);
  }

  // when pipe is finished redirect stdin to terminal to ensure blocking getch
  freopen("/dev/tty", "r", stdin);
  wclear(app.win);
  if (game_check_finished(&app.game)) {
    app.msg = "game over. congratulations!\n"
                  "press 'q' to quit";
    wattron(app.win, COLOR_PAIR(GREEN));
  } else {
    wattron(app.win, COLOR_PAIR(RED));
    mvwprintw(app.win, 0, 0,
              "game stil not over. but your moves are.\n"
              "press 'q' to quit");
  }
  mvwprintw(app.win, 0, 0, "%s", app.msg);
  wrefresh(app.win);
  int ch;
  while ((ch = wgetch(app.win))) {
    if (ch == 'q') {
      endwin();
      break;
    }
  }
  return EXIT_SUCCESS;
}

void app_play_line(app_t* app, char* line, char**prevmsg, int* failed_sum)
{
  int failed = 0;
  char * msg = NULL;
  napms(app->tick_time_ms);
  if (strcmp(line, "move\n") == 0) {
    failed = game_move_player(&app->game);
    msg = "move";
  } else if (strcmp(line, "rotate\n") == 0) {
    failed = game_rotate_player(&app->game, 1);
  } else if (strcmp(line, "rotate -90\n") == 0) {
    failed = game_rotate_player(&app->game, -1);
  } else if (strcmp(line, "light\n") == 0) {
    failed = game_light(&app->game);
    msg = "light";
  }
  if (failed && msg != NULL) {
    if (*prevmsg == NULL || strcmp(msg, *prevmsg) == 0) {
      *failed_sum+=1;
    } else {
      *failed_sum=1;
    }
    int size = 18+strlen(msg)+intlen(*failed_sum);
    app->msg = malloc(sizeof(char) * size);
    sprintf(app->msg, "\n%s failed %d time(s)", msg, *failed_sum);
    mvwprintw(app->win, 0, 0, "%s", app->msg);
    *prevmsg=msg;
    wrefresh(app->win);
  }
  #if __DEBUG
  fprintf(stderr, "info: cmd got \"%s\"\n", line);
  #endif // __DEBUG
}

void app_init_level(app_t* app, char** argv, int argc)
{
  if (app->level_generator) {
    // 3=2 spaces, one \0
    int len = strlen(app->level_generator) + intlen(app->maxy) + intlen(app->maxx) + 3;
    char* cmd = malloc(sizeof(char) * len);
    sprintf(cmd, "'%s' %d %d", app->level_generator, app->maxy, app->maxx);
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
      die("fatal: failed running generator command");
    }
    char buffer[4096];
    int size = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
      size += strlen(buffer);
      if (app->level == NULL) {
        app->level = malloc(size+1);
      } else {
        char* tmp = realloc(app->level, size+1);
        if (tmp) {
          app->level = tmp;
        }
      }
      strcat(app->level, buffer);
    }
    pclose(pipe);
  } else {
    FILE* fp = fopen(argv[optind++], "r");
    if (fp == NULL) {
      die("fatal: failed opening file");
    }
    fseek(fp, 0, SEEK_END);
    size_t length;
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    app->level = malloc(sizeof(char) * length);
    if (app->level)
    {
      fread(app->level, 1, length, fp);
    }
    fclose(fp);
  }
}

void handle_winch(int sig)
{
  endwin();
  // refresh and clear to initialize dimensions
  refresh();
  clear();
  game_redraw(&app.game);
  getmaxyx(stdscr, app.maxy, app.maxx);
  #if __DEBUG
  fprintf(stderr, "x: %d, y: %d\n", app.maxx, app.maxy);
  #endif //__DEBUG
  wresize(app.win, 2, app.maxx);
  if (app.msg) {
    mvwprintw(app.win, 0, 0, "%s", app.msg);
  }
  wrefresh(app.win);
}

int intlen(int n)
{
  int len = 1;
  while (n/=10) {
    len++;
  }
  return len;
}

