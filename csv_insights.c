#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIME_PERIOD_MS 100
#define FALSE 0
#define TRUE 1
#define VOLT_MAX 5000
#define MIN_POWER 0
#define VOLT_DROP_THRESHOLD 4900
#define MAX_ROWS 10000
#define LITRES_PERWATT_PERHOUR 0.00045336481

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("Incorrect number of arguments\n");
    return -1;
  }
  int voltage_vals[MAX_ROWS];
  for (int i = 0; i < argc; i++)
  {
    printf("%s\n", argv[i]);
  }
  int start = atoi(argv[2]), end = atoi(argv[3]);
  int num_periods = end - start;
  double time_elapsed = num_periods * TIME_PERIOD_MS / 1000.0;
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
  double current, power;
  int lowest_drop = VOLT_MAX;
  double max_power = MIN_POWER;

  int total_drop_length = 0, drop_count = 0, total_drop_min = 0;
  double total_power = 0, total_useful_power = 0;
  int useful_power_counter = 0;

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
    power = voltage * current / 1000; // W
    total_power += power;
    if (power > 0)
    {
      total_useful_power += power;
      useful_power_counter++;
    }
    if (power > max_power)
      max_power = power;
    printf("I=%03.0lfmA, P=%04.0lfW\n", current, power);
    if (voltage < VOLT_DROP_THRESHOLD && !in_drop)
    {
      drop_start = i;
      drop_min = VOLT_MAX;
      in_drop = TRUE;
      printf("Start voltage drop. Index: %d, Voltage: %d\n", i, voltage);
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
  fclose(fp);

  fp = fopen(argv[4], "w");

  fprintf(fp, "This trial lasted for %.0lfs.\n", time_elapsed);
  fprintf(fp, "The voltage dropped %d times and the average drop length was %.1lfs.\n", drop_count, total_drop_length / (double)drop_count / 1000);
  fprintf(fp, "The average drop fell to %.0lfV.\n", total_drop_min / (double)drop_count);
  fprintf(fp, "The lowest the voltage dropped was %dV.\n", lowest_drop);
  fprintf(fp, "The average power consumption during this trial was %.1lfW.\n", total_power / (float)num_periods);
  fprintf(fp, "Counting only the power when zapping, the average power is %.1lfW.\n", total_useful_power / useful_power_counter);
  fprintf(fp, "The maximum power reached during this trial was %.1lfW.\n", max_power);
  fprintf(fp, "The average fuel consumption is %.2lfL/hr.", total_power / (float)num_periods * LITRES_PERWATT_PERHOUR);

  fclose(fp);
  return 0;
}