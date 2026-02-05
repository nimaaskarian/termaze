#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>


int parse_times(const char ** input);
void parse_input(const char * input);

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fputs("usage: thecodinggame <level-file>\n", stderr);
    return EXIT_FAILURE;
  }
  FILE* fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fputs("error: failed opening file\n", stderr);
    return EXIT_FAILURE;
  }
  char * buffer = 0;
  size_t length;
  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buffer = malloc(sizeof(char) * length);
  if (buffer)
  {
    fread (buffer, 1, length, fp);
  }
  fclose (fp);
  parse_input(buffer);
  return EXIT_SUCCESS;
}

enum direction_t {
  up,
  right,
  down,
  left,
};

void parse_input(const char * input) {
  enum direction_t direction = up;
  for (;*input; input++) {
    switch (*input) {
      case 'u': 
        direction = up;
        puts("changed direction to up");
        break;
      case 'd': 
        direction = down;
        puts("changed direction to down");
        break;
      case 'r': 
        direction = right;
        puts("changed direction to right");
        break;
      case 'l': 
        direction = left;
        puts("changed direction to left");
        break;
      case 'm':
        {
          int n = parse_times(&input);
          printf("%d times move with direction %d\n", n, direction);
          // make move n times in direction
        }
      case 'P':
        {
          int n = parse_times(&input);
          printf("%d times path with direction %d\n", n, direction);
          // make path n times in direction
        }
      break;
      case 'L':
        {
          int n = parse_times(&input);
          printf("%d times light with direction %d\n", n, direction);
          // make light n times in direction
        }
      break;
      case ' ':
      case '\n':
      case '\r':
      case '\t':
      continue;
      default:
      fprintf(stderr, "warning: invalid char %c skipped\n", *input);
      break;
    }
  }
}

int parse_times(const char ** input) {
  int i = 1;
  if ((*input)[i] == '*') {
    if (!isdigit((*input)[++i])) {
      fprintf(stderr, "error: expected number after *. found another character '%c'\n", *input[i]);
      exit(1);
    }
    while (isdigit((*input)[i])) i++;
    char * num_str = malloc(sizeof(char)*(i-1));
    strncpy(num_str, (*input)+2, i-2);
    num_str[i-2] = 0;
    *input+=i;
    return atoi(num_str);
  } else {
    return 1;
  }
}
