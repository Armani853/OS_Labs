#include "../include/functions.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        const char* usage = "Использование: ./lab2_run <число_массивов> <длина_каждого> <макс_потоков>\n";
        safe_write_err(usage);
        return 1;
    }

    int k = atoi(argv[1]);
    int n = atoi(argv[2]);
    int max_threads = atoi(argv[3]);

    if (k <= 0 || k > MAX_ARRAYS) {
        const char* err = "Ошибка: число массивов должно быть от 1 до ";
        safe_write_err(err);
        write_int(MAX_ARRAYS);
        safe_write_err("\n");
        return 1;
    }
    if (n <= 0 || n > MAX_LEN) {
        const char* err = "Ошибка: длина массива должна быть от 1 до ";
        safe_write_err(err);
        write_int(MAX_LEN);
        safe_write_err("\n");
        return 1;
    }
    if (max_threads <= 0) {
        const char* err = "Ошибка: число потоков должно быть положительным\n";
        safe_write_err(err);
        return 1;
    }

    int logical_cores = sysconf(_SC_NPROCESSORS_ONLN);
    const char* cores_msg = "Обнаружено логических ядер: ";
    safe_write(cores_msg);
    write_int(logical_cores);
    safe_write("\n");

    double **arrays = malloc(k * sizeof(double*));
    double *result_seq = malloc(n * sizeof(double));
    double *result_par = malloc(n * sizeof(double));

    if (!arrays || !result_seq || !result_par) {
        const char* err = "Ошибка выделения памяти\n";
        safe_write_err(err);
        return 1;
    }

    for (int i = 0; i < k; i++) {
        arrays[i] = malloc(n * sizeof(double));
        if (!arrays[i]) {
            const char* err = "Ошибка выделения памяти для массива\n";
            safe_write_err(err);
            return 1;
        }
    }

    srand(time(NULL));
    init_arrays(arrays, k, n);

    print_input_arrays(arrays, k, n);

    long long start = get_time_ms();
    sequential_sum(arrays, result_seq, k, n);
    long long seq_time = get_time_ms() - start;

    print_result_array("Sequential", result_seq, n);

    const char* seq_msg = "Последовательное время: ";
    safe_write(seq_msg);
    write_number(seq_time);
    const char* ms = " мс\n";
    safe_write(ms);

    long long* par_times = calloc(max_threads, sizeof(long long));
    if (!par_times) {
        safe_write_err("Ошибка выделения памяти для par_times\n");
        return 1;
    }

    for (int t = 1; t <= max_threads; t++) {
        start = get_time_ms();
        parallel_sum(arrays, result_par, k, n, t);
        par_times[t - 1] = get_time_ms() - start;

        if (!validate_results(result_seq, result_par, n, 1e-6)) {
            const char* err = "Ошибка: результаты не совпадают при ";
            safe_write_err(err);
            write_int(t);
            const char* threads = " потоках!\n";
            safe_write_err(threads);
            break;
        }
    }

    print_result_array("Parallel", result_par, n);

    print_performance_table(max_threads, seq_time, par_times);

    for (int i = 0; i < k; i++) {
        free(arrays[i]);
    }
    free(arrays);
    free(result_seq);
    free(result_par);
    free(par_times);

    return 0;
}