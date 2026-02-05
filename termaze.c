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
  init_pair(BLACK, COLOR_WHITE, COLOR_BLACK);
  init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);
  init_pair(GREEN, COLOR_WHITE, COLOR_GREEN);
  init_pair(YELLOW, COLOR_WHITE, COLOR_YELLOW);
  init_pair(RED, COLOR_WHITE, COLOR_RED);
  init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
}

game_t game_init(char *input)
{
  ncurses_init();
  lines_t lines = get_lines(input);
  player_t player = lines_draw_ncurses_return_player(lines);
  game_t out = {.player = player, .lines = lines};
  return out;
}

#define PRINT_LIGHT(y, x) {\
  attron(COLOR_PAIR(WHITE));\
  mvprintw(y, x, " ");\
}

#define PRINT_LIGHT_ON(y, x) {\
  attron(COLOR_PAIR(YELLOW));\
  mvprintw(y, x, " ");\
}

#define PRINT_PATH(y, x) {\
  attron(COLOR_PAIR(GREEN));\
  mvprintw(y, x, " ");\
}

#define PRINT_CHAR(Y, X, C) {\
    switch (C) {\
      case CHAR_PLEFT:\
      case CHAR_PRIGHT:\
      case CHAR_PUP:\
      case CHAR_PDOWN:\
      case CHAR_PATH:\
        PRINT_PATH(Y, X);\
      break;\
      case CHAR_LIGHT:\
        PRINT_LIGHT(Y, X);\
      break;\
    }\
}

// helper for game_move_player
#define PRINT_CUR_CHAR() {\
    char ch = game->lines.buff[game->player.y][game->player.x];\
    PRINT_CHAR(game->player.y, game->player.x, ch);\
}

// give -1 to rotate for counter clockwise, and 1 for clockwise
void player_rotate(player_t* player, int rotate)
{
  player->dir+=rotate;
  if (player->dir == DIR_MIN) {
    player->dir = DIR_MAX-1;
  }
  if (player->dir == DIR_MAX) {
    player->dir = DIR_MIN+1;
  }
  print_player(player);
}

// returns EXIT_SUCCESS (0) on success. EXIT_FAILIURE otherwise
int game_move_player(game_t* game)
{
  switch (game->player.dir) {
    case up:
      if (game->player.y == 0 
        || game->lines.buff[game->player.y-1][game->player.x] == ' ') {
        return EXIT_FAILURE;
      }
      PRINT_CUR_CHAR();
      game->player.y--;
    break;
    case down:
      if (game->player.y >= game->lines.size 
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
      if (game->player.x >= game->lines.sizes[game->player.y] 
        || game->lines.buff[game->player.y][game->player.x+1] == ' ') {
        return EXIT_FAILURE;
      }
      PRINT_CUR_CHAR();
      game->player.x++;
    break;
  }
  print_player(&game->player);
  return EXIT_SUCCESS;
}

#define PLAYER_PRINT(S) mvprintw(player->y, player->x, S)
void print_player(player_t* player)
{
  attron(COLOR_PAIR(BLACK));
  switch (player->dir) {
    case down:
    PLAYER_PRINT("↓");
    break;
    case right:
    PLAYER_PRINT("→");
    break;
    case left:
    PLAYER_PRINT("←");
    break;
    case up:
    PLAYER_PRINT("↑");
    break;
  }
}

#define PLAYER_INIT(DIR) {\
if (player_defined) {\
  fprintf(stderr, "Player already defined. line %d:%d\n", x,y);\
  exit(1);\
}\
player.x = x;\
player.y = y;\
player.dir = DIR;\
player_defined = true;\
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
      fprintf(stderr, "error: max line size exeeded. this shouldn't have happend. if it did, shame the author in https://github.com/nimaaskarian/termaze/issues\n");
      exit(1);
    }
    buff[i] = strtok(NULL, "\n");
  }
  size_t* sizes = malloc(sizeof(size_t)*i);
  for (int j = 0; j < i; j++) {
    sizes[j] = strlen(buff[j]);
  }
  lines_t out = {.buff=buff, .size=i, sizes=sizes};
  return out;
}

player_t lines_draw_ncurses_return_player(lines_t lines)
{
  player_t player;
  bool player_defined = false;
  for (int y = 0; y < lines.size; y++) {
    char* line = lines.buff[y];
    for (int x = 0; x < lines.sizes[y]; x++) {
    fprintf(stderr, "x: %d, y: %d\n", x, y);
      switch (line[x]) {
        case CHAR_PATH:
          PRINT_PATH(y, x);
        break;
        case CHAR_LIGHT:
          PRINT_LIGHT(y, x);
        break;
        case CHAR_PRIGHT:
          PLAYER_INIT(right)
        break;
        case CHAR_PUP:
          PLAYER_INIT(up)
        break;
        case CHAR_PDOWN:
          PLAYER_INIT(down)
        break;
        case CHAR_PLEFT:
          PLAYER_INIT(left)
        break;
        case CHAR_IGNORE:
        break;
        default:
        fprintf(stderr, "warning: invalid char '%c' replaced with '%c'", *line, CHAR_IGNORE);
        line[x] = ' ';
      }
    }
  }
  print_player(&player);
  return player;
}
