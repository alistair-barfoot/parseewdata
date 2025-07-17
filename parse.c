#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  FILE *in;
  in = fopen("ew_data.txt", "r");

  FILE *out;
  out = fopen("trial2_parsed.csv", "w");

  char str[100];
  fgets(str, sizeof(str), in);
  fprintf(out, "%s", str);
  int index = 0;

  if (in != NULL)
  {
    while (fgets(str, sizeof(str), in))
    {

      if (str[9] == '5' || str[12] == '5' || (str[12] == '6' && str[14] == '0'))
        continue;
      index++;
      if (index < 8268)
        continue;
      if (index > (1474 + 8268) && index < (8268 + 1981 + 18))
        continue;
      int i = 0;
      for (; str[i] != '.'; i++)
        ;
      str[i] = ',';
      for (; str[i] != '-' && str[i] != '\n'; i++)
        ;
      if (str[i] == '-')
        continue;
      printf("%s", str);
      fprintf(out, "%s", str);
    }
  }
  else
  {
    printf("File failed to open.\n");
    return 1;
  }
  fclose(out);
  fclose(in);

  return 0;
}