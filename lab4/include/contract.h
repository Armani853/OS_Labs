#ifndef CONTRACT_H
#define CONTRACT_H

#include <stddef.h>

typedef char*(*ConvertFunc)(int);

typedef int*(*SortFunc)(int* array, size_t n);

#endif 