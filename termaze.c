#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <locale.h>
#include "termaze.h"
#include "util.h"

void ncurses_init()
{
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, false);
  start_color();
  init_pair(BLACK, COLOR_WHITE, COLOR_BLACK);
  init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);
  init_pair(GREEN, COLOR_BLACK, COLOR_GREEN);
  init_pair(YELLOW, COLOR_BLACK, COLOR_YELLOW);
  init_pair(RED, COLOR_BLACK, COLOR_RED);
  init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
}

void game_free(game_t* game)
{
  delwin(game->win);
  FREE(game->lines.sizes);
  FREE(game->lines.buff);
}

game_t game_init(char *input)
{
  DEBUG("game init started\n");
  lines_t lines = get_lines(input);
  player_t player = parse_lines_return_player(lines);
  game_t game = {.player = player, .lines = lines};
  game.win = newwin(lines.size, lines.max_size, 0, 0);
  game_redraw(&game);
  DEBUG("game init successful\n");
  return game;
}

// helper
#define gameprintonplayer(game, S) {\
  mvwprintw(game->win, game->player.y, game->player.x, S);\
}

// helper for game_move_player
#define PRINT_CUR_CHAR() {\
    char ch = game->lines.buff[game->player.y][game->player.x];\
    ATTR_CHAR(ch);\
    gameprintonplayer(game, " ");\
    wrefresh(game->win);\
}

// give -1 to rotate for counter clockwise, and 1 for clockwise
int game_rotate_player(game_t* game, int rotate)
{
  game->player.dir+=rotate;
  if (game->player.dir <= DIR_MIN) {
    game->player.dir = DIR_MAX-1;
  }
  if (game->player.dir >= DIR_MAX) {
    game->player.dir = DIR_MIN+1;
  }
  game_print_player(game);
  return EXIT_SUCCESS;
}

int game_light(game_t* game)
{
  switch (game->lines.buff[game->player.y][game->player.x]) {
    case LIGHT:
      game->lines.buff[game->player.y][game->player.x] = LIGHT_ON;
    break;
    case LIGHT_ON:
      game->lines.buff[game->player.y][game->player.x] = LIGHT;
    break;
    default:
      return EXIT_FAILURE;
  }
  game_print_player(game);
  return EXIT_SUCCESS;
}

bool game_check_finished(game_t* game)
{
  for (int y = 0; y < game->lines.size; y++) {
    for (int x = 0; x < game->lines.sizes[y]; x++) {
      if (game->lines.buff[y][x] == LIGHT) {
        return false;
      }
    }
  }
  return true;
}

#define die_dir_min_max_cases()                                                \
  case DIR_MIN:                                                                \
  case DIR_MAX:                                                                \
    die("fatal: invalid direction (%d) at %s:%d.\n"                            \
        "this shouldn't have happened. post the level (or level "              \
        "generator) and the "                                                  \
        "script produced the stdin at "                                        \
        "https://github.com/nimaaskarian/termaze/issues",                      \
        game->player.dir, __FILE__, __LINE__);                                 \
    break;

// returns EXIT_SUCCESS (0) on success. EXIT_FAILIURE otherwise
int game_move_player(game_t* game)
{
  switch (game->player.dir) {
    die_dir_min_max_cases();
    case up:
      if (game->player.y == 0 
        || game->lines.buff[game->player.y-1][game->player.x] == ' ') {
        return EXIT_FAILURE;
      }
      PRINT_CUR_CHAR();
      game->player.y--;
    break;
    case down:
      if (game->player.y+1 >= game->lines.size 
        || game->lines.buff[game->player.y+1][game->player.x] == ' ') {
        return EXIT_FAILURE;
      }
      PRINT_CUR_CHAR();
      game->player.y++;
    break;
    case left:
      if (game->player.x == 0 
        || game->lines.buff[game->player.y][game->player.x-1] == ' ') {
        return EXIT_FAILURE;
      }
      PRINT_CUR_CHAR();
      game->player.x--;
    break;
    case right:
      if (game->player.x+1 >= game->lines.sizes[game->player.y] 
        || game->lines.buff[game->player.y][game->player.x+1] == ' ') {
        return EXIT_FAILURE;
      }
      PRINT_CUR_CHAR();
      game->player.x++;
    break;
  }
  game_print_player(game);
  return EXIT_SUCCESS;
}

void game_print_player(game_t* game)
{
  ATTR_CHAR(game->lines.buff[game->player.y][game->player.x]);
  switch (game->player.dir) {
    die_dir_min_max_cases();
    case down:
    gameprintonplayer(game, S_PLAYER_DOWN);
    break;
    case right:
    gameprintonplayer(game, S_PLAYER_RIGHT);
    break;
    case left:
    gameprintonplayer(game, S_PLAYER_LEFT);
    break;
    case up:
    gameprintonplayer(game, S_PLAYER_UP);
    break;
  }
  wrefresh(game->win);
}

#define PLAYER_INIT(DIR) {\
if (player_defined) {\
  die("error: player already defined. line %d:%d", x,y);\
}\
player.x = x;\
player.y = y;\
player.dir = DIR;\
player_defined = true;\
line[x] = 'P';\
}

// TODO: use \n\r in windows
lines_t get_lines(char* input)
{
  size_t len = strlen(input);
  int newline_count = 0;
  for (int i = 0; i < len; i++) {
    if (input[i] == '\n') {
      newline_count++;
    }
  }
  char** buff = malloc(sizeof(char *)*newline_count);
  size_t* sizes = malloc(sizeof(size_t) * newline_count);
  int max_size = 0;

  buff[0] = strtok(input, "\n");
  sizes[0] = strlen(buff[0]);
  for (int i = 1; i < newline_count; i ++) {
    buff[i] = strtok(NULL, "\n");
    if (buff[i] == NULL) {
      break;
    }
    DEBUG("BUFF %s, %d\n", buff[i], i);
    sizes[i] = strlen(buff[i]);
    if (sizes[i] > max_size) {
      max_size = sizes[i];
    }
  }

  lines_t out = {.buff=buff, .size=newline_count, sizes=sizes, .max_size=max_size};
  return out;
}

void game_redraw(game_t* game)
{
  for (int y = 0; y < game->lines.size; y++) {
    char* line = game->lines.buff[y];
    for (int x = 0; x < game->lines.sizes[y]; x++) {
      ATTR_CHAR(line[x]);
      mvwprintw(game->win, y, x, " ");\
    }
  }
  game_print_player(game);
}

player_t parse_lines_return_player(lines_t lines)
{
  player_t player = {.x=0, .y=0, .dir=DIR_MIN+1};
  bool player_defined = false;
  for (int y = 0; y < lines.size; y++) {
    char* line = lines.buff[y];
    for (int x = 0; x < lines.sizes[y]; x++) {
      DEBUG("x: %d, y: %d\n", x, y);
      switch (line[x]) {
        case PATH:
        break;
        case LIGHT:
        break;
        case PLAYER_RIGHT:
          PLAYER_INIT(right)
        break;
        case PLAYER_UP:
          PLAYER_INIT(up)
        break;
        case PLAYER_DOWN:
          PLAYER_INIT(down)
        break;
        case PLAYER_LEFT:
          PLAYER_INIT(left)
        break;
        case IGNORE:
        break;
        default:
        DEBUG("warning: invalid char '%c' replaced with '%c'", *line, IGNORE);
        line[x] = IGNORE;
      }
    }
  }
  return player;
}
