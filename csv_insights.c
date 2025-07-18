#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

// #define DEBUG // Uncomment to enable debug print statements

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

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

// Struct to hold cumulative data
typedef struct
{
  double total_power;
  double total_useful_power;
  double max_power;
  int useful_power_counter;
  int total_drop_length;
  int total_drop_min;
  int drop_count;
  int lowest_drop;
} PowerStats;

/**
 * Skip a given number of lines in the file
 */
void skipLines(FILE *fp, int lines)
{
  char buffer[100];
  for (int i = 0; i < lines; i++)
  {
    fgets(buffer, sizeof(buffer), fp);
  }
}

/**
 * Extracts voltage or current value from a CSV line based on the column index
 */
double extractValue(char *line, int column)
{
  int commas_seen = 0;
  int i = 0;

  while (line[i] != '\0' && commas_seen < column)
  {
    if (line[i] == ',')
      commas_seen++;
    i++;
  }

  char value_str[16] = {0};
  int j = 0;
  while (line[i] != ',' && line[i] != '\n' && line[i] != '\0' && j < 15)
  {
    value_str[j++] = line[i++];
  }
  value_str[j] = '\0';
  return atof(value_str);
}

/**
 * Processes a CSV line and updates statistics accordingly
 */
void processLine(char *line, int index, PowerStats *stats, int *in_drop, int *drop_start)
{
  double voltage = extractValue(line, VOLTAGE_COL);
  double current = extractValue(line, CURRENT_COL);
  double power = voltage * current / 1000.0;

  stats->total_power += power;
  if (power > 0)
  {
    stats->total_useful_power += power;
    stats->useful_power_counter++;
  }
  if (power > stats->max_power)
  {
    stats->max_power = power;
  }
  if (voltage < stats->lowest_drop)
  {
    stats->lowest_drop = voltage;
  }

  if (voltage < VOLT_DROP_THRESHOLD && !(*in_drop))
  {
    *drop_start = index;
    *in_drop = TRUE;
    stats->total_drop_min += VOLT_MAX;
    DEBUG_PRINT("Start drop at index %d, V=%.0lf\n", index, voltage);
  }

  if (*in_drop)
  {
    if (voltage < stats->total_drop_min)
    {
      stats->total_drop_min = voltage;
    }
    if (voltage >= VOLT_DROP_THRESHOLD)
    {
      int drop_length = (1 + index - *drop_start) * TIME_PERIOD_MS;
      stats->total_drop_length += drop_length;
      stats->drop_count++;
      *in_drop = FALSE;
      DEBUG_PRINT("Drop ended at index %d, length=%dms\n", index, drop_length);
    }
  }
}

/**
 * Calculates and prints output to a results file
 */
void printToFile(const char *output_file, double time_elapsed, int distance_travelled, PowerStats stats)
{
  FILE *fp = fopen(output_file, "w");

  double avg_drop_len = stats.total_drop_length / (double)stats.drop_count / 1000;
  double avg_drop_val = stats.total_drop_min / (double)stats.drop_count;
  double avg_power = stats.total_power / (stats.useful_power_counter + (stats.total_power == 0 ? 1 : 0));
  double avg_useful = stats.total_useful_power / (double)stats.useful_power_counter;
  double avg_fuel = avg_power * LITRES_PERWATT_PERHOUR;
  double total_energy = avg_power * time_elapsed / 1000;
  double energy_per_dist = total_energy / distance_travelled;

  fprintf(fp, "This trial lasted for %.0lfs and travelled %dm.\n", time_elapsed, distance_travelled);
  fprintf(fp, "The voltage dropped %d times and the average drop length was %.1lfs.\n", stats.drop_count, avg_drop_len);
  fprintf(fp, "The average drop fell to %.0lfV.\n", avg_drop_val);
  fprintf(fp, "The lowest the voltage dropped was %dV.\n", stats.lowest_drop);
  fprintf(fp, "The average power consumption during this trial was %.1lfW.\n", avg_power);
  fprintf(fp, "Counting only the power when zapping, the average power is %.1lfW.\n", avg_useful);
  fprintf(fp, "The maximum power reached during this trial was %.1lfW.\n", stats.max_power);
  fprintf(fp, "The average fuel consumption is %.2lfL/hr.\n", avg_fuel);
  fprintf(fp, "The total amount of energy consumed during this trial was %.1lfkJ.\n", total_energy);
  fprintf(fp, "The energy used per unit distance is %.2lfkJ/m.\n", energy_per_dist);

  fclose(fp);
}

/**
 * Validates command-line arguments and returns 0 if valid
 */
int validateArgs(int argc, char *argv[])
{
  if (argc != 6)
  {
    printf("Incorrect number of arguments\n");
    return FALSE;
  }
  int start = atoi(argv[2]);
  int end = atoi(argv[3]);
  if ((end - start) > MAX_ROWS)
  {
    printf("Too many rows.\n");
    return FALSE;
  }
  return TRUE;
}

int main(int argc, char *argv[])
{
  if (!validateArgs(argc, argv))
    return -1;

  int start = atoi(argv[2]);
  int end = atoi(argv[3]);
  int distance_travelled = atoi(argv[4]);
  char *outputFile = argv[5];
  int num_periods = end - start;
  double time_elapsed = num_periods * TIME_PERIOD_MS / 1000.0;

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL)
  {
    printf("File location not found\n");
    return -1;
  }

  skipLines(fp, start);

  PowerStats stats = {0};
  stats.lowest_drop = VOLT_MAX;
  int in_drop = FALSE, drop_start = 0;

  char line[100];
  for (int i = start; i < end; i++)
  {
    if (fgets(line, sizeof(line), fp))
    {
      processLine(line, i, &stats, &in_drop, &drop_start);
    }
  }

  fclose(fp);
  printToFile(outputFile, time_elapsed, distance_travelled, stats);
  return 0;
}