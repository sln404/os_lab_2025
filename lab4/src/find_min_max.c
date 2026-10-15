#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = begin; i < end; i++) {
    int cur = array[i];
    if (min_max.max < cur)
      min_max.max = cur;
    if (min_max.min > cur)
      min_max.min = cur;
  }

  return min_max;
}
