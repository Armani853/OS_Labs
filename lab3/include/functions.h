#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <ctype.h>
#include <sys/wait.h>
#include <time.h>
#include "status_codes.h"

#define MAX_LINE_LENGTH 1024
#define MAX_FILENAME_LENGTH 256
#define FILTER_LENGTH 10
#define SHM_SIZE MAX_LINE_LENGTH

void remove_vowels(char* str);

void generate_unique_name(char *buffer, size_t buf_size, const char *prefix);