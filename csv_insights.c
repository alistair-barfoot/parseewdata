#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    printf("Incorrect number of arguments\n");
    return -1;
  }
  int voltage_vals[10000];
  for (int i = 0; i < argc; i++)
  {
    printf("%s\n", argv[i]);
  }

  return 0;
}