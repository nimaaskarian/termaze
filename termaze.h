#ifndef TERMAZE_H
#define TERMAZE_H
#define MAX_LINE_SIZE 4096
#include <ncurses.h>
#include <stdlib.h>
#define CHAR_PATH 'P'
#define CHAR_LIGHT 'L'
#define CHAR_LIGHT_ON 'O'
#define CHAR_IGNORE ' '
#define CHAR_PLEFT '<'
#define CHAR_PRIGHT '>'
#define CHAR_PDOWN 'v'
#define CHAR_PUP '^'
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
void game_print_player(game_t* game);
int game_move_player(game_t* game);
void game_rotate_player(game_t* game, int rotate);
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
