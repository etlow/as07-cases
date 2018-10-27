#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INPUTS(NAME) "inputs/" NAME ".%ld.in"
#define OUTPUTS(NAME) "outputs/" NAME ".%ld.out"
#define PEAK "peak"
#define SCRIPTS "scripts"
#define INVERSION "inversion"
#define DEF_CASES 20
#define DEF_LENGTH 20

// Code modified from Unit 22 - Efficiency
long count_inversion(long i, long n, const long rank[n])
{
  long count = 0;
  for (long j = i + 1; j < n; j += 1) {
    if (rank[i] > rank[j]) {
      count ++;
    }
  }
  return count;
}

long kendall_tau(long n, const long rank[n])
{
  long count = 0;
  for (long i = 0; i < n - 1; i += 1) {
    count += count_inversion(i, n, rank);
  }
  return count;
}

long generate_peak(long n, long unordered[])
{
  long curr = -1;
  long k;
  long direction = rand() % 8;
  long peak = rand() % 3;
  if (direction == 0) {
    k = n;
  } else if (direction == 1) {
    k = 0;
  } else {
    k = rand() % (n - 2) + 1;
  }
  for (long i = 0; i < k; i += 1) {
    curr += rand() % 3;
    unordered[i] = curr;
  }
  unordered[k] = curr + peak;
  for (long i = k + 1; i < n; i += 1) {
    curr -= rand() % 3;
    unordered[i] = curr;
  }
  if (peak > 0 && direction > 1) {
    return k;
  } else {
    return 0;
  }
}

void generate_ids(long n, long ordered[], long unordered[], bool working[])
{
  long curr = -2;
  for (long i = 0; i < n; i += 1) {
    curr += rand() % 3 + 1;
    ordered[i] = curr;
  }
  for (long i = 0; i < n; i += 1) {
    working[i] = true;
  }
  long i = 0;
  for (long index = rand() % 3; index < n; index += rand() % 3 + 1) {
    working[index] = false;
    unordered[i] = ordered[index];
    i += 1;
  }
  for (long index = n - 1; index >= 0; index -= 1) {
    if (working[index]) {
      unordered[i] = ordered[index];
      i += 1;
    }
  }
}

FILE *open_file(char *path, long test_num)
{
  char filename[50];
  snprintf(filename, sizeof(char) * 50, path, test_num);
  int fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    return NULL;
  } else {
    return fdopen(fd, "w");
  }
}

void close_file(FILE *file)
{
  if (file != NULL) {
    fclose(file);
  }
}

void write_list(FILE *file, long length, long list[])
{
  for (long i = 0; i < length; i += 1) {
    // printf("%3ld", list[i]);
    fprintf(file, "%ld ", list[i]);
  }
  // printf("\n");
  fprintf(file, "\n");
}

void write_peak(long length, long unordered[], long i)
{
  FILE *in = open_file(INPUTS(PEAK), i);
  FILE *out = open_file(OUTPUTS(PEAK), i);
  if (in != NULL && out != NULL) {
    printf(INPUTS(PEAK)"\n", i);
    long output = generate_peak(length, unordered);
    fprintf(in, "%ld\n", length);
    write_list(in, length, unordered);
    if (output == 0) {
      fprintf(out, "no peak\n");
    } else {
      fprintf(out, "%ld\n", output);
      // printf("%2ld\n", output);
    }
  }
  close_file(in);
  close_file(out);
}

void write_scripts_inversion(long length, long ordered[], long unordered[], 
    bool working[], long i)
{
  bool scripts = false;
  bool inversion = false;
  FILE *scripts_in = open_file(INPUTS(SCRIPTS), i);
  FILE *scripts_out = open_file(OUTPUTS(SCRIPTS), i);
  FILE *inversion_in = open_file(INPUTS(INVERSION), i);
  FILE *inversion_out = open_file(OUTPUTS(INVERSION), i);
  if (scripts_in != NULL && scripts_out != NULL) {
    scripts = true;
  }
  if (inversion_in != NULL && inversion_out != NULL) {
    inversion = true;
  }
  if (scripts || inversion) {
    generate_ids(length, ordered, unordered, working);
  }
  if (scripts) {
    printf(INPUTS(SCRIPTS)"\n", i);
    fprintf(scripts_in, "%ld\n", length);
    write_list(scripts_in, length, unordered);
    write_list(scripts_out, length, ordered);
  }
  if (inversion) {
    printf(INPUTS(INVERSION)"\n", i);
    fprintf(inversion_in, "%ld\n", length);
    write_list(inversion_in, length, unordered);
    fprintf(inversion_out, "%ld\n", kendall_tau(length, unordered));
  }
  close_file(scripts_in);
  close_file(scripts_out);
  close_file(inversion_in);
  close_file(inversion_out);
}

long convert_check_decimal(char *str, char *name, long def)
{
  long num = strtol(str, NULL, 10);
  if (num < 1) {
    printf("%s is %ld, using %ld\n", name, num, def);
    return def;
  } else {
    return num;
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  long num_cases = DEF_CASES;
  long max_length = DEF_LENGTH;
  if (argc > 1) {
    num_cases = convert_check_decimal(argv[1], "Number of cases", DEF_CASES);
    if (argc > 2) {
      max_length = convert_check_decimal(argv[2], "Max length", DEF_LENGTH);
      if (argc > 3) {
        printf("%d arguments received, ignoring last %d\n", argc - 1, argc - 3);
      }
    }
  }
  long *ordered = malloc(max_length * sizeof(long));
  if (ordered == NULL) {
    printf("Error allocating ordered array");
    return 0;
  }
  long *unordered = malloc(max_length * sizeof(long));
  if (unordered == NULL) {
    printf("Error allocating unordered array");
    free(ordered);
    return 0;
  }
  bool *working = malloc(max_length * sizeof(bool));
  if (working == NULL) {
    printf("Error allocating working array");
    free(ordered);
    free(unordered);
    return 0;
  }
  long peak_max = max_length;
  if (peak_max < 3) {
    printf("Max length for peak is at least 3\n");
    peak_max = 3;
  }
  for (long i = 1; i <= num_cases; i += 1) {
    write_peak(rand() % (peak_max - 2) + 3, unordered, i);
    write_scripts_inversion(rand() % max_length + 1, ordered, unordered, working, i);
  }
}
