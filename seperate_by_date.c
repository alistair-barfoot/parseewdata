#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MTHCOL 1
#define DAYCOL 2
#define VOLTCOL 7
#define CURRCOL 8

double extractValue(char *line, int column)
{
  int commas_seen = 0;
  int i = 0;

  // Move index forward until the desired number of commas have been seen
  while (line[i] != '\0' && commas_seen < column)
  {
    if (line[i] == ',')
      commas_seen++;
    i++;
  }

  // Extract characters starting at the column of interest until the next comma/newline
  char value_str[16]; // Buffer to hold the numeric string
  int j = 0;
  while (line[i] != ',' && line[i] != '\n' && line[i] != '\0' && j < 15)
  {
    value_str[j++] = line[i++];
  }
  value_str[j] = '\0';

  // Convert string to a floating point number
  return atof(value_str);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Incorrect number of arguments.\n");
    return -1;
  }
  else
  {
    printf("Seperating data from file %s...\n", argv[1]);
  }
  FILE *in = fopen(argv[1], "r");
  if (in == NULL)
  {
    printf("File location not found. \n");
    return -1;
  }

  char str[100];
  char header[100];
  fgets(header, sizeof(str), in);
  int date;

  for (int i = 1; i <= 31; i++)
  {
    fgets(str, sizeof(str), in);
    int month = extractValue(str, MTHCOL);
    date = extractValue(str, DAYCOL);
    if (date != i)
    {
      continue;
    }
    char filename[50];
    sprintf(filename, "output_%d-%d.csv", month, i);
    FILE *out = fopen(filename, "w");
    fprintf(out, "%s", header);
    do
    {
      fgets(str, sizeof(str), in);
      date = extractValue(str, DAYCOL);
      int error = extractValue(str, CURRCOL);
      if (error < 0)
        printf("%d\n", error);

      if (date == i)
      {
        fprintf(out, "%s", str);
      }
      else
      {
        break;
      }
    } while (1);
    fclose(out);
  }
  fclose(in);

  return 0;
}