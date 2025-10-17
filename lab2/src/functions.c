#include "../include/functions.h"

void safe_write(const char* msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void safe_write_err(const char* msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

void write_number(long long num) {
    char buffer[BUFFER_SIZE];
    int len = snprintf(buffer, sizeof(buffer), "%lld", num);
    if (len > 0 && len < BUFFER_SIZE) {
        write(STDOUT_FILENO, buffer, len);
    }
}

void write_int(int num) {
    char buffer[BUFFER_SIZE];
    int len = snprintf(buffer, sizeof(buffer), "%d", num);
    if (len > 0 && len < BUFFER_SIZE) {
        write(STDOUT_FILENO, buffer, len);
    }
}

void write_double(double val) {
    char buffer[BUFFER_SIZE];
    int len = snprintf(buffer, sizeof(buffer), "%.6f", val);
    if (len > 0 && len < BUFFER_SIZE) {
        write(STDOUT_FILENO, buffer, len);
    }
}

void write_speedup(double speedup) {
    char buffer[BUFFER_SIZE];
    int len = snprintf(buffer, sizeof(buffer), "%.2f", speedup);
    if (len > 0 && len < BUFFER_SIZE) {
        write(STDOUT_FILENO, buffer, len);
    }
}

void write_efficiency(double efficiency) {
    char buffer[BUFFER_SIZE];
    int len = snprintf(buffer, sizeof(buffer), "%.2f%%", efficiency * 100);
    if (len > 0 && len < BUFFER_SIZE) {
        write(STDOUT_FILENO, buffer, len);
    }
}

long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void init_arrays(double **arrays, int k, int n) {
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            arrays[i][j] = (double)(rand() % 1000) / 100.0;
        }
    }
}

void sequential_sum(double **arrays, double *result, int k, int n) {
    for (int j = 0; j < n; j++) {
        result[j] = 0.0;
        for (int i = 0; i < k; i++) {
            result[j] += arrays[i][j];
        }
    }
}

void* thread_sum(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int j = data->start_idx; j < data->end_idx; j++) {
        data->result[j] = 0.0;
        for (int i = 0; i < data->k; i++) {
            data->result[j] += data->arrays[i][j];
        }
    }
    return NULL;
}

void parallel_sum(double **arrays, double *result, int k, int n, int num_threads) {
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    int chunk = n / num_threads;
    int remainder = n % num_threads;

    for (int t = 0; t < num_threads; t++) {
        thread_data[t].arrays = arrays;
        thread_data[t].result = result;
        thread_data[t].k = k;
        thread_data[t].n = n;
        thread_data[t].start_idx = t * chunk + (t < remainder ? t : remainder);
        thread_data[t].end_idx = (t + 1) * chunk + ((t + 1) < remainder ? (t + 1) : remainder);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_create(&threads[t], NULL, thread_sum, &thread_data[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

int validate_results(double *seq, double *par, int n, double epsilon) {
    for (int i = 0; i < n; i++) {
        if (fabs(seq[i] - par[i]) > epsilon) {
            return 0;
        }
    }
    return 1;
}

void print_performance_table(int max_threads, long long seq_time, long long* par_times) {
    const char* header = "\nЧисло потоков | Время исполнения (мс) | Ускорение | Эффективность\n"
                         "-------------|------------------------|-----------|--------------\n";
    safe_write(header);

    for (int t = 1; t <= max_threads; t++) {
        double speedup = (double)seq_time / par_times[t - 1];
        double efficiency = speedup / t;

        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, sizeof(buffer),
                           "%12d | %22lld | %9.2f | %11.2f%%\n",
                           t, par_times[t - 1], speedup, efficiency * 100);
        if (len > 0 && len < BUFFER_SIZE) {
            write(STDOUT_FILENO, buffer, len);
        }
    }
}