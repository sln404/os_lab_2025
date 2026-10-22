#ifndef SUM_H
#define SUM_H

#include <stdlib.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

size_t Sum(const struct SumArgs *args);

#endif