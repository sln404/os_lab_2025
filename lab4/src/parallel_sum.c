#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>
#include <limits.h>

#include "utils.h"
#include "sum.h"

void* ThreadSum(void* args) {
  struct SumArgs* sum_args = (struct SumArgs*)args;
  return (void*)(size_t)Sum(sum_args);
}

int main(int argc, char** argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  bool is_print_array = false;

  static struct option options[] = {
    {"threads_num", required_argument, 0, 0},
    {"seed", required_argument, 0, 0},
    {"array_size", required_argument, 0, 0},
    {"print_array", no_argument, 0, 0},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  while (true) {
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1) 
      break;
    switch (option_index) {
      case 0: 
        threads_num = atoi(optarg);
        break;
      case 1: 
        seed = atoi(optarg);
        break;
      case 2: 
        array_size = atoi(optarg);
        break;
      case 3: 
        is_print_array = true;
        break;
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (threads_num == 0 || array_size == 0 || seed == 0) {
    printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\" [--print_array] \n", argv[0]);
    return 1;
  }

  pthread_t threads[threads_num];
  int* array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    printf("Failed to allocate memory for array\n");
    return 2;
  }

  GenerateArray(array, array_size, seed);

  if (is_print_array) {
    for (uint32_t i = 0; i < array_size; i++)
    {
      printf("%u)\t%d\n", i, array[i]);
    }
    printf("\n");
  }

  struct SumArgs args[threads_num];
  struct timeval start_time, finish_time;

  gettimeofday(&start_time, NULL);

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * (array_size / threads_num);
    if (i == threads_num - 1) {
      args[i].end = array_size;
    } else {
      args[i].end = (i + 1) * (array_size / threads_num);
    }

    if (pthread_create(&threads[i], NULL, ThreadSum, (void*)&args[i]) != 0) {
      printf("Failed to create thread\n");
      return 1;
    }
  }

  size_t total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    size_t sum = 0;
    if (pthread_join(threads[i], (void**)&sum) != 0) {
      printf("Failed to join thread");
      return 1;
    }
    total_sum += sum;
  }

  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  printf("Sum: %lu\n", total_sum);
  printf("Elapsed time: %fms\n", elapsed_time);
  return 0;
}
