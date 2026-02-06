#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <locale.h>
#include "termaze.h"

void ncurses_init()
{
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  start_color();
  nodelay(stdscr, true);
  init_pair(BLACK, COLOR_WHITE, COLOR_BLACK);
  init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);
  init_pair(GREEN, COLOR_BLACK, COLOR_GREEN);
  init_pair(YELLOW, COLOR_BLACK, COLOR_YELLOW);
  init_pair(RED, COLOR_WHITE, COLOR_RED);
  init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
}

game_t game_init(char *input)
{
  lines_t lines = get_lines(input);
  player_t player = parse_lines_return_player(lines);
  game_t game = {.player = player, .lines = lines};
  game.win = newwin(lines.size, lines.max_size, 0, 0);
  game_redraw(&game);
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
void game_rotate_player(game_t* game, int rotate)
{
  game->player.dir+=rotate;
  if (game->player.dir <= DIR_MIN) {
    game->player.dir = DIR_MAX-1;
  }
  if (game->player.dir >= DIR_MAX) {
    game->player.dir = DIR_MIN+1;
  }
  game_print_player(game);
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
  }
  game_print_player(game);
  return EXIT_FAILURE;
}

#define die_dir_min_max_cases() case DIR_MIN:\
    case DIR_MAX:\
      endwin();\
      fprintf(stderr,\
              "invalid direction (%d) at %s:%d.\n"\
              "this shouldn't have happened. post the level (or level "\
              "generator) and the "\
              "script produced the stdin at https://github.com/nimaaskarian/termaze/issues",\
              game->player.dir, __FILE__, __LINE__);\
      exit(1);\
    break;\

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
  endwin();\
  fprintf(stderr, "error: player already defined. line %d:%d\n", x,y);\
  exit(1);\
}\
player.x = x;\
player.y = y;\
player.dir = DIR;\
player_defined = true;\
line[x] = 'P';\
}

// TODO: use \n\r in windows. also fix the fixed size line size
lines_t get_lines(char* input)
{
  char** buff = malloc(sizeof(char *[MAX_LINE_SIZE]));
  int i = 0;
  buff[i] = strtok(input, "\n");
  while (buff[i]) {
    i++;
    if (i >= MAX_LINE_SIZE) {
      endwin();
      fprintf(stderr, "error: max line size exeeded. this shouldn't have "
                      "happend. if it did, shame the author in "
                      "https://github.com/nimaaskarian/termaze/issues\n");
      exit(1);
    }
    buff[i] = strtok(NULL, "\n");
  }
  size_t* sizes = malloc(sizeof(size_t)*i);
  int max_size = 0;
  for (int j = 0; j < i; j++) {
    sizes[j] = strlen(buff[j]);
    if (sizes[j] > max_size) {
      max_size = sizes[j];
    }
  }
  lines_t out = {.buff=buff, .size=i, sizes=sizes, .max_size=max_size};
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
      #if __DEBUG
      fprintf(stderr, "x: %d, y: %d\n", x, y);
      #endif /* if __DEBUG */
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
        #if __WARN | __DEBUG
        fprintf(stderr, "warning: invalid char '%c' replaced with '%c'", *line, IGNORE);
        #endif /* if __WARN | __DEBUG */
        line[x] = IGNORE;
      }
    }
  }
  return player;
}
