#include <stdio.h>
#include "command.h"

int main(int argc, char *argv[])
{
  // ./redir <input> <command> <output>
  if (argc != 4)
  {
    printf("Usage: ./redir <input> <command> <output>");
    return 1;
  }
  char * input = argv[1];
  char * cmd = argv[2];
  char * output = argv[3];
  FILE * in = fopen(input, "r");
  FILE * out = fopen(output, "w");

  char * result = command(cmd, in);
  fclose(in);
  fprintf(out, "%s", result);
  fclose(out);
  return 0;
}
