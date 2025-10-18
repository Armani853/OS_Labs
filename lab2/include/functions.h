#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define MAX_ARRAYS 1000
#define MAX_LEN    1000000
#define BUFFER_SIZE 256
#define DISPLAY_LIMIT 10

typedef struct {
    double **arrays;
    double *result;
    int k;
    int n;
    int start_idx;
    int end_idx;
} ThreadData;

void safe_write(const char* msg);

void safe_write_err(const char* msg);

void write_number(long long num);

void write_int(int num);

void write_double(double val);

void write_speedup(double speedup);

void write_efficiency(double efficiency);

long long get_time_ms();

void init_arrays(double **arrays, int k, int n);

void sequential_sum(double **arrays, double *result, int k, int n);

void* thread_sum(void* arg);

void parallel_sum(double **arrays, double *result, int k, int n, int num_threads);

int validate_results(double *seq, double *par, int n, double epsilon);

void print_performance_table(int max_threads, long long seq_time, long long* par_times);

void print_input_arrays(double **arrays, int k, int n);

void print_result_array(const char* label, double *result, int n);

