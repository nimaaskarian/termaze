#ifndef TERMAZE_H
#define TERMAZE_H
#define MAX_LINE_SIZE 4096
#include <stdlib.h>
typedef enum direction {
  up = 0,
  right,
  down,
  left,
} direction_t;

typedef struct player {
  int x,y;
  direction_t dir;
} player_t;

typedef struct lines {
  char** buff;
  size_t size;
} lines_t;

lines_t get_buff(char* input);
player_t parse_level(lines_t lines);

enum color {
  WHITE = 1,
  GREEN,
  YELLOW,
  RED,
  BLUE,
  BLACK,
};
#endif // !TERMAZE_H
