#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

// Shouldn't need to change these values
#define VOLTAGE_COL 7
#define CURRENT_COL 8

// Change these values based on what type of experiment
#define TIME_PERIOD_MS 100       // ms
#define VOLT_MAX 5000            // V
#define VOLT_DROP_THRESHOLD 4900 // V
#define MIN_POWER 0              // W
#define MAX_ROWS 10000

// Based on Honda 7000i ES
// 19.2L lasts for 12.1h at 50% load (3.5kW)
#define FUEL_CAPACITY 19.2  // L
#define RUN_LENGTH 12.1     // h
#define GENERATOR_LOAD 3500 // W
#define LITRES_PERWATT_PERHOUR FUEL_CAPACITY / RUN_LENGTH / GENERATOR_LOAD

void printToFile(char *output_file, double time_elapsed, int distance_travelled, int drop_count, double average_drop_length, double average_drop, int lowest_drop, double average_power, double average_useful_power, double max_power, double average_fuel, double total_energy, double energy_per_dist)
{
  FILE *fp = fopen(output_file, "w");
  printf("%s\n", output_file);

  fprintf(fp, "This trial lasted for %.0lfs and travelled %dm.\n", time_elapsed, distance_travelled);
  fprintf(fp, "The voltage dropped %d times and the average drop length was %.1lfs.\n", drop_count, average_drop_length);
  fprintf(fp, "The average drop fell to %.0lfV.\n", average_drop);
  fprintf(fp, "The lowest the voltage dropped was %dV.\n", lowest_drop);
  fprintf(fp, "The average power consumption during this trial was %.1lfW.\n", average_power);
  fprintf(fp, "Counting only the power when zapping, the average power is %.1lfW.\n", average_useful_power);
  fprintf(fp, "The maximum power reached during this trial was %.1lfW.\n", max_power);
  fprintf(fp, "The average fuel consumption is %.2lfL/hr.\n", average_fuel);
  fprintf(fp, "The total amount of energy consumed during this trial was %.1lfkJ.\n", total_energy);
  fprintf(fp, "The energy used per unit distance is %.2lfkJ/m.\n", energy_per_dist);

  fclose(fp);

  return;
}

int main(int argc, char *argv[])
{
  if (argc != 6)
  {
    printf("Incorrect number of arguments\n");
    return -1;
  }
  int start = atoi(argv[2]), end = atoi(argv[3]);
  char *outputFile = argv[5];

  if ((end - start) > MAX_ROWS)
  {
    printf("Too many rows.\n");
    return -1;
  }

  int voltage_vals[MAX_ROWS];
  for (int i = 0; i < argc; i++)
  {
    printf("%s\n", argv[i]);
  }

  int num_periods = end - start;
  double time_elapsed = num_periods * TIME_PERIOD_MS / 1000.0;
  printf("%.1lf\n", time_elapsed);
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

  char voltage_str[10], current_str[10];
  int current_start, voltage_start;
  int drop_min;
  int in_drop = FALSE;
  int drop_start = 0;
  double voltage, current, power;
  int lowest_drop = VOLT_MAX;
  double max_power = MIN_POWER;

  int total_drop_length = 0, drop_count = 0, total_drop_min = 0;
  double total_power = 0, total_useful_power = 0;
  int useful_power_counter = 0;

  for (int i = start; i < end; i++)
  {
    fgets(str, sizeof(str), fp);

    int comma_count = 0;
    for (voltage_start = 0; comma_count < VOLTAGE_COL; ++voltage_start)
    {
      if (str[voltage_start] == ',')
        comma_count++;
    }
    for (int j = 0; str[j] != ','; j++)
    {
      voltage_str[j] = str[j + voltage_start];
    }
    voltage = atof(voltage_str);
    comma_count = 0;
    for (current_start = 0; comma_count < CURRENT_COL; ++current_start)
    {
      if (str[current_start] == ',')
        comma_count++;
    }
    for (int j = 0; str[j + 1] != '\n'; j++)
    {
      current_str[j] = str[j + current_start];
    }
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
    // printf("V=%lfV,I=%03.0lfmA, P=%04.0lfW\n", voltage, current, power);
    if (voltage < VOLT_DROP_THRESHOLD && !in_drop)
    {
      drop_start = i;
      drop_min = VOLT_MAX;
      in_drop = TRUE;
      // printf("Start voltage drop. Index: %d, Voltage: %d\n", i, voltage);
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
      int drop_length = (1 + i - drop_start) * TIME_PERIOD_MS;
      // printf("The voltage dropped for %d ms and the lowest value was %dV.\n", drop_length, drop_min);
      total_drop_length += drop_length;
      total_drop_min += drop_min;
      drop_count++;
      in_drop = FALSE;
    }
  }
  fclose(fp);

  int distance_travelled = atoi(argv[4]);
  double average_drop_length = total_drop_length / (double)drop_count / 1000;
  double average_drop = total_drop_min / (double)drop_count;
  double average_power = total_power / (float)num_periods;
  double average_useful_power = total_useful_power / useful_power_counter;
  double average_fuel = total_power / (float)num_periods * LITRES_PERWATT_PERHOUR;
  double total_energy = time_elapsed * total_power / (float)num_periods / 1000;
  double energy_per_dist = time_elapsed * total_power / (float)num_periods / 1000 / distance_travelled;

  printToFile(outputFile, time_elapsed, distance_travelled, drop_count, average_drop_length, average_drop, lowest_drop, average_power, average_useful_power, max_power, average_fuel, total_energy, energy_per_dist);

  return 0;
}