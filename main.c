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
  char * solver;

  game_t game;
  int maxx, maxy;
  WINDOW* win;
  char* msg;
  bool msg_malloced;
  char* level;
  char* leveldup;
} app_t;

void app_init_level(app_t* app, char** argv, int argc);
void app_print_quit(app_t* app);
void app_play_line(app_t* app, char line[256], char**prevmsg, int* failed_sum);
void handle_winch(int sig);
int intlen(int n);

app_t app;

int main(int argc, char *argv[])
{
  int c;
  app.tick_time_ms = 100;
  app.level_generator = NULL;
  app.solver = NULL;
  app.msg = NULL;
  app.level = NULL;
  app.leveldup = NULL;
  while (true) {
    static struct option long_options[] = {
        {"tick-time", required_argument, 0, 't'},
        {"level-generator", required_argument, 0, 'g'},
        {"solver", required_argument, 0, 's'},
        {0, 0, 0, 0}};

    c = getopt_long (argc, argv, "t:g:s:", long_options, 0);
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
      case 's':
      app.solver = optarg;
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
  app.win = newwin(2, app.maxx, app.maxy-2, 0);
  while (true) {
    if (app.solver) {
      app.leveldup = strdup(app.level);
    }
    app.game = game_init(app.level);

    char line[256];
    int failed_sum = 0;
    char * prevmsg = NULL;
    if (app.solver) {
      FILE* pipe = popen(app.solver, "r");
      if (!pipe) {
        die("fatal: failed running solver command");
      }
      while (fgets(line, sizeof(line), pipe) != NULL) {
        app_play_line(&app, line, &prevmsg, &failed_sum);
      }
      pclose(pipe);
    } else {
      while (fgets(line, sizeof(line), stdin)) {
        app_play_line(&app, line, &prevmsg, &failed_sum);
      }
    }

    // when pipe is finished redirect stdin to terminal to ensure blocking getch
    freopen("/dev/tty", "r", stdin);
    wclear(app.win);
    FREE(app.msg);
    if (game_check_finished(&app.game)) {
      app.msg = "game over. congratulations!";
      wattron(app.win, COLOR_PAIR(GREEN));
    } else {
      wattron(app.win, COLOR_PAIR(RED));
      app.msg = "game stil not over. but your moves are.";
    }
    mvwprintw(app.win, 0, 0, "%s", app.msg);
    app_print_quit(&app);
    wrefresh(app.win);
    int ch;
    while ((ch = wgetch(app.win))) {
      if (ch == 'q') {
        game_free(&app.game);
        app.msg = NULL;
        FREE(app.level);
        delwin(app.win);
        endwin();
        return EXIT_SUCCESS;
      }
      if (app.solver && ch == 'r') {
        game_free(&app.game);
        app.msg = NULL;
        FREE(app.level);
        app.level = app.leveldup;
        wclear(app.win);
        wrefresh(app.win);
        break;
      }
    }
  }
}

void app_print_quit(app_t* app)
{
  if (app->solver) {
    mvwprintw(app->win, 1, 0, "press 'q' to quit, 'r' to restart");
  } else {
    mvwprintw(app->win, 1, 0, "press 'q' to quit.");
  }
}

void app_play_line(app_t* app, char line[256], char**prevmsg, int* failed_sum)
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
    FREE(app->msg);
    app->msg = malloc(sizeof(char) * size);
    app->msg_malloced = true;
    wattron(app->win, COLOR_PAIR(BLACK));
    sprintf(app->msg, "\n%s failed %d time(s)", msg, *failed_sum);
    mvwprintw(app->win, 0, 0, "%s", app->msg);
    *prevmsg=msg;
    wrefresh(app->win);
  }
  DEBUG("info: cmd got \"%s\"\n", line);
}

void app_init_level(app_t* app, char** argv, int argc)
{
  if (app->level_generator) {
    // 3=2 spaces, one \0
    int len = strlen(app->level_generator) + intlen(app->maxy) + intlen(app->maxx) + 3;
    char* cmd = malloc(sizeof(char) * len);
    sprintf(cmd, "'%s' %d %d", app->level_generator, app->maxy, app->maxx);
    FILE* pipe = popen(cmd, "r");
    free(cmd);
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
  DEBUG("x: %d, y: %d\n", app.maxx, app.maxy);
  wresize(app.win, 2, app.maxx);
  if (app.msg) {
    mvwprintw(app.win, 0, 0, "%s", app.msg);
  }
  app_print_quit(&app);
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

