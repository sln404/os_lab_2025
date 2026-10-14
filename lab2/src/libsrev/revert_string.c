#include "stddef.h"

void Swap(char *left, char *right)
{
    char temp;
    temp = *left;
    *left = *right;
    *right = temp;
}

void RevertString(char *str)
{
    if (str == NULL) 
		return;

    char *left = str;
    char *right = str;

    while (*right) {
        right++;
    }
    right--;

    while (left < right) {
        Swap(left, right);
        left++;
        right--;
    }
}
