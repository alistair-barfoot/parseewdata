#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIME_PERIOD_MS 100
#define FALSE 0
#define TRUE 1
#define VOLT_MAX 5000
#define VOLT_DROP_THRESHOLD 4900

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
  int start = atoi(argv[2]), end = atoi(argv[3]);
  FILE *fp = fopen(argv[1], "r");
  printf("%s\n", argv[1]);

  if (fp == NULL)
  {
    printf("File location not found\n");
    return -1;
  }

  char str[100];

  for (int i = 0; i < start; i++)
  {
    fgets(str, sizeof(str), fp);
  }

  char voltage_str[5], current_str[10];
  int current_start;
  int voltage, drop_min;
  int in_drop = FALSE;
  int drop_start = 0;
  double current;
  int lowest_drop = VOLT_MAX;

  int total_drop_length = 0, drop_count = 0, total_drop_min = 0;

  for (int i = start; i < end; i++)
  {
    fgets(str, sizeof(str), fp);
    for (int j = 0; j < 4; j++)
    {
      voltage_str[j] = str[j + 24];
    }
    int comma_count = 0;
    for (current_start = 0; comma_count < 8; ++current_start)
    {
      if (str[current_start] == ',')
        comma_count++;
    }
    for (int j = 0; j < 9 && str[j + 1] != '\n'; j++)
    {
      current_str[j] = str[j + current_start];
    }
    // printf("%s\n", current_str);
    voltage = atoi(voltage_str);
    current = atof(current_str);
    printf("I=%.2lf\n", current);
    if (voltage < VOLT_DROP_THRESHOLD && !in_drop)
    {
      drop_start = i;
      drop_min = VOLT_MAX;
      in_drop = TRUE;
      printf("Index: %d, Voltage: %d\n", i, voltage);
    }
    if (in_drop && voltage < drop_min)
    {
      drop_min = voltage;
    }
    if (voltage < lowest_drop)
    {
      lowest_drop = voltage;
    }
    if (in_drop && voltage >= VOLT_DROP_THRESHOLD)
    {
      printf("The voltage dropped for %d ms and the lowest value was %dV.\n", (i - drop_start) * TIME_PERIOD_MS, drop_min);
      total_drop_length += (i - drop_start) * TIME_PERIOD_MS;
      total_drop_min += drop_min;
      drop_count++;
      in_drop = FALSE;
    }
  }

  printf("The voltage dropped %d times and the average drop length was %.1lfs.\n", drop_count, total_drop_length / (double)drop_count / 1000);
  printf("The average drop fell to %.0lfV.\n", total_drop_min / (double)drop_count);
  printf("The lowest the voltage dropped was %dV.\n", lowest_drop);
  return 0;
}