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
  init_pair(GREEN, COLOR_BLACK, COLOR_GREEN);
  init_pair(YELLOW, COLOR_BLACK, COLOR_YELLOW);
  init_pair(RED, COLOR_WHITE, COLOR_RED);
  init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
}

game_t game_init(char *input)
{
  ncurses_init();
  lines_t lines = get_lines(input);
  player_t player = lines_draw_ncurses_return_player(lines);
  game_t game = {.player = player, .lines = lines};
  game_print_player(&game);
  return game;
}

#define ATTR_ON_LIGHT() {\
  attron(COLOR_PAIR(WHITE));\
}

#define ATTR_ON_LIGHT_ON() {\
  attron(COLOR_PAIR(YELLOW));\
}

#define ATTR_ON_PATH() {\
  attron(COLOR_PAIR(GREEN));\
}

#define ATTR_ON_CHAR(C) {\
    switch (C) {\
      case CHAR_PATH:\
        ATTR_ON_PATH();\
      break;\
      case CHAR_LIGHT:\
        ATTR_ON_LIGHT();\
      break;\
      case CHAR_LIGHT_ON:\
        ATTR_ON_LIGHT_ON();\
      break;\
    }\
}

// helper for game_move_player
#define PRINT_CUR_CHAR() {\
    char ch = game->lines.buff[game->player.y][game->player.x];\
    ATTR_ON_CHAR(ch);\
    mvprintw(game->player.y, game->player.x, " ");\
}

// give -1 to rotate for counter clockwise, and 1 for clockwise
void game_rotate_player(game_t* game, int rotate)
{
  game->player.dir+=rotate;
  if (game->player.dir == DIR_MIN) {
    game->player.dir = DIR_MAX-1;
  }
  if (game->player.dir == DIR_MAX) {
    game->player.dir = DIR_MIN+1;
  }
  game_print_player(game);
}

int game_light(game_t* game)
{
  if (game->lines.buff[game->player.y][game->player.x] == 'L') {
    game->lines.buff[game->player.y][game->player.x] = 'O';
    game_print_player(game);
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
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
  game_print_player(game);
  return EXIT_SUCCESS;
}

#define PLAYER_PRINT(S) mvprintw(game->player.y, game->player.x, S)
void game_print_player(game_t* game)
{
  ATTR_ON_CHAR(game->lines.buff[game->player.y][game->player.x]);
  switch (game->player.dir) {
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
      #if __DEBUG
      fprintf(stderr, "x: %d, y: %d\n", x, y);
      #endif /* if __DEBUG */
      switch (line[x]) {
        case CHAR_PATH:
          ATTR_ON_PATH();
          mvprintw(y, x, " ");
        break;
        case CHAR_LIGHT:
          ATTR_ON_LIGHT();
          mvprintw(y, x, " ");
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
        #if __WARN | __DEBUG
        fprintf(stderr, "warning: invalid char '%c' replaced with '%c'", *line, CHAR_IGNORE);
        #endif /* if __WARN | __DEBUG */
        line[x] = ' ';
      }
    }
  }
  return player;
}
