#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids = NULL;
int pnum = -1;

void kill_children(int sig) {
    if (child_pids != NULL) {
        for (int i = 0; i < pnum; i++) {
            if (child_pids[i] > 0) {
                kill(child_pids[i], SIGKILL);
            }
        }
        printf("\nTimeout exceeded. Killing child processes...\n");
    }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) 
      break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed is a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size is a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
             if (pnum <= 0) {
              printf("pnum is a positive number\n.\n");
              return 1;
            }
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout is a positive number\n.\n");
              return 1;
            } 
            break;
          case 4:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files | -f] [--timeout \"num\"]\n",
           argv[0]);
    return 1;
  }

  if (pnum > array_size) {
    pnum = array_size;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
      printf("Failed to allocate memory for array\n");
      return 2;
  }
  
  GenerateArray(array, array_size, seed);

  int active_child_processes = 0;

  int (*pipes)[2] = NULL;

  if (!with_files) {
    pipes = malloc(sizeof(int[2]) * pnum);
    if (pipes == NULL) {
      printf("Failed to allocate memory for pipes\n");
      free(array);
      return 2;
    }
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) == -1) {
        perror("pipe");
        free(array);
        free(pipes);
        return 2;
      }
    }
  }

  child_pids = malloc(sizeof(pid_t) * pnum);
  if (child_pids == NULL) {
      printf("Failed to allocate memory for child pids\n");
      return 2;
  }

  if (timeout > 0) {
    signal(SIGALRM, kill_children);
    alarm(timeout);
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {

        int chunk_size = array_size / pnum;
        int start = i * chunk_size;
        int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;

        struct MinMax local_min_max = GetMinMax(array, start, end);

        if (with_files) {
          char filename[32];
          sprintf(filename, "child_%d.tmp", i);
          FILE *f = fopen(filename, "wb");
          if (f == NULL) {
            perror("fopen child file");
            free(array);
            exit(1);
          }
          fwrite(&local_min_max, sizeof(struct MinMax), 1, f);
          fclose(f);
        } else {
          close(pipes[i][0]);

          if (write(pipes[i][1], &local_min_max, sizeof(struct MinMax)) == -1) {
            perror("write to pipe");
            close(pipes[i][1]);
            free(array);
            free(pipes);
            exit(1);
          }
          
          close(pipes[i][1]);
        }

        free(array);
        if (!with_files) 
          free(pipes);
        exit(0);
      } else {
          child_pids[i] = child_pid;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (!with_files) {
    for (int i = 0; i < pnum; i++) {
      close(pipes[i][1]);
    }
  }

  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    struct MinMax local_min_max;

    if (with_files) {
      char filename[32];
      sprintf(filename, "child_%d.tmp", i);
      FILE *f = fopen(filename, "rb");
      if (f == NULL) {
          perror("fopen child file for reading");
          continue;
      }
      fread(&local_min_max, sizeof(struct MinMax), 1, f);
      fclose(f);
      remove(filename);

    } else {
      if (read(pipes[i][0], &local_min_max, sizeof(struct MinMax)) == -1) {
          perror("read from pipe");
          continue;
      }
      close(pipes[i][0]);
    }

    if (local_min_max.min < min_max.min) 
      min_max.min = local_min_max.min;
    if (local_min_max.max > min_max.max) 
      min_max.max = local_min_max.max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);
  if (!with_files) {
    free(pipes);
  }

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
