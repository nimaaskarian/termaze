#ifndef TERMAZE_H
#define TERMAZE_H
#define MAX_LINE_SIZE 4096
#include <stdlib.h>
#define CHAR_PATH 'P'
#define CHAR_LIGHT 'L'
#define CHAR_IGNORE ' '
#define CHAR_PLEFT '<'
#define CHAR_PRIGHT '>'
#define CHAR_PDOWN 'v'
#define CHAR_PUP '^'
typedef enum direction {
  DIR_MIN=-1,
  up = 0,
  right,
  down,
  left,
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
} lines_t;

typedef struct game {
  player_t player;
  lines_t lines;
} game_t;

lines_t get_lines(char* input);
game_t game_init(char* input);
void print_player(player_t* player);
int game_move_player(game_t* game);
void ncurses_init();
player_t lines_draw_ncurses_return_player(lines_t lines);
void player_rotate(player_t* player, int rotate);

enum color {
  WHITE = 1,
  GREEN,
  YELLOW,
  RED,
  BLUE,
  BLACK,
};
#endif // !TERMAZE_H
