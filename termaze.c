#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "termaze.h"

#define PLAYER_PRINT(S) mvprintw(player->y, player->x, S)
void print_player(player_t* player) {
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
x++;\
player_defined = true;\
}

// TODO: use \n\r in windows. also fix the fixed size line size
lines_t get_buff(char* input)
{
  char** buff = malloc(sizeof(char *[MAX_LINE_SIZE]));
  char * line = strtok(input, "\n");
  int i = 0;
  while (line) {
    if (i >= MAX_LINE_SIZE) {
      fprintf(stderr, "error: max line size exeeded. this shouldn't have happend. if it did, shame the author in https://github.com/nimaaskarian/termaze/issues\n");
      exit(1);
    }
    buff[i++] = strtok(input, "\n");
  }
  lines_t out = {.buff=buff, .size=i};
  return out;
}

player_t parse_level(lines_t lines)
{
  player_t player;
  bool player_defined = false;
  for (int y = 0;y < lines.size; y++) {
    int x = 0;
    char* line = lines.buff[y];
    for (;*line; line++) {
      switch (*line) {
        case 'P':
          attron(COLOR_PAIR(GREEN));
          mvprintw(y, x, " ");
          x+=1;
        break;
        case 'L':
          attron(COLOR_PAIR(WHITE));
          mvprintw(y, x, " ");
          x+=1;
        break;
        case '>':
          PLAYER_INIT(right)
        break;
        case '^':
          PLAYER_INIT(up)
        break;
        case 'v':
          PLAYER_INIT(down)
        break;
        case '<':
          PLAYER_INIT(left)
        break;
        case ' ':
          x+=1;
        break;
        case '\n':
          y+=1;
          x=0;
        break;
      }

    }
  }
  print_player(&player);
  return player;
}
