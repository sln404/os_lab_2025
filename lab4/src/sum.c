#include "sum.h"

size_t Sum(const struct SumArgs *args) {
  size_t sum = 0;
  if (args->array == 0) 
    return 0;

  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}