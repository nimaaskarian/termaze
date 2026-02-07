#ifndef TERMAZE_H
#define TERMAZE_H
#define MAX_LINE_SIZE 4096
#include <ncurses.h>
#include <stdlib.h>
#define PATH 'P'
#define LIGHT 'L'
#define LIGHT_ON 'O'
#define IGNORE ' '
#define PLAYER_RIGHT '>'
#define PLAYER_DOWN 'v'
#define PLAYER_LEFT '<'
#define PLAYER_UP '^'
#define S_PLAYER_RIGHT "→"
#define S_PLAYER_DOWN "↓"
#define S_PLAYER_LEFT "←"
#define S_PLAYER_UP "↑"

#define ATTR_LIGHT COLOR_PAIR(WHITE)
#define ATTR_LIGHT_ON COLOR_PAIR(YELLOW)
#define ATTR_PATH COLOR_PAIR(GREEN)
#define ATTR_IGNORE COLOR_PAIR(BLACK)

#define ATTR_CHAR(C) {\
    switch (C) {\
      case IGNORE:\
        wattron(game->win, ATTR_IGNORE);\
      break;\
      case PATH:\
        wattron(game->win, ATTR_PATH);\
      break;\
      case LIGHT:\
        wattron(game->win, ATTR_LIGHT);\
      break;\
      case LIGHT_ON:\
        wattron(game->win, ATTR_LIGHT_ON);\
      break;\
    }\
}

typedef enum direction {
  DIR_MIN=-1,
  right,
  down,
  left,
  up,
  DIR_MAX,
} direction_t;

typedef struct player {
  int x,y;
  direction_t dir;
} player_t;

typedef struct lines {
  char** buff;
  size_t size;
  size_t* sizes;
  int max_size;
} lines_t;

typedef struct game {
  player_t player;
  lines_t lines;
  WINDOW* win;
} game_t;

void ncurses_init();
lines_t get_lines(char* input);
void game_redraw(game_t* game);
player_t parse_lines_return_player(lines_t lines);
game_t game_init(char* input);
bool game_check_finished(game_t* game);
void game_print_player(game_t* game);
int game_move_player(game_t* game);
int game_rotate_player(game_t* game, int rotate);
int game_light(game_t* game);

enum color {
  WHITE = 1,
  GREEN,
  YELLOW,
  RED,
  BLUE,
  BLACK,
};
#endif // !TERMAZE_H
