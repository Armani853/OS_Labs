#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/contract.h"
#include <dlfcn.h> 
#include <unistd.h>
#include <stdarg.h>

static ConvertFunc current_convert = NULL;
static SortFunc current_sort = NULL;

static void *lib_handle_1_v1 = NULL;
static void *lib_handle_1_v2 = NULL;
static void *lib_handle_2_v1 = NULL;
static void *lib_handle_2_v2 = NULL;

static int current_impl_1 = 1;
static int current_impl_2 = 1; 

#define LIB1_V1_PATH "bin/libconvert_bin.so"
#define LIB1_V2_PATH "bin/libconvert_tri.so"
#define LIB2_V1_PATH "bin/libsort_bubble.so"
#define LIB2_V2_PATH "bin/libsort_hoare.so"

#define MAX_OUTPUT_BUFFER 512

static void custom_write(int fd, const char *format, ...) {
    char buffer[MAX_OUTPUT_BUFFER];
    va_list args;
    va_start(args, format);
    int len = snprintf(buffer, sizeof(buffer), "%s", format);
    va_end(args);
    
    if (len > 0) {
        write(fd, buffer, (size_t)len);
    }
}

static int custom_read_line(char *buffer, size_t max_len) {
    ssize_t bytes_read = read(STDIN_FILENO, buffer, max_len - 1);
    if (bytes_read <= 0) {
        return 0;
    }
    buffer[bytes_read] = '\0';
    
    char *newline = strchr(buffer, '\n');
    if (newline) {
        *newline = '\0';
    }
    
    return (int)bytes_read;
}


static int load_libraries() {
    lib_handle_1_v1 = dlopen(LIB1_V1_PATH, RTLD_LAZY);
    lib_handle_1_v2 = dlopen(LIB1_V2_PATH, RTLD_LAZY);
    lib_handle_2_v1 = dlopen(LIB2_V1_PATH, RTLD_LAZY);
    lib_handle_2_v2 = dlopen(LIB2_V2_PATH, RTLD_LAZY);

    if (!lib_handle_1_v1 || !lib_handle_1_v2 || !lib_handle_2_v1 || !lib_handle_2_v2) {
        char err_msg[MAX_OUTPUT_BUFFER];
        snprintf(err_msg, sizeof(err_msg), "Ошибка загрузки одной из библиотек: %s\n", dlerror());
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        return 0;
    }

    current_convert = (ConvertFunc)dlsym(lib_handle_1_v1, "convert");
    current_sort = (SortFunc)dlsym(lib_handle_2_v1, "sort");
    
    if (!current_convert || !current_sort) {
        char err_msg[MAX_OUTPUT_BUFFER];
        snprintf(err_msg, sizeof(err_msg), "Ошибка поиска символа 'convert' или 'sort': %s\n", dlerror());
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        return 0;
    }

    return 1;
}

static void unload_libraries() {
    if (lib_handle_1_v1) dlclose(lib_handle_1_v1);
    if (lib_handle_1_v2) dlclose(lib_handle_1_v2);
    if (lib_handle_2_v1) dlclose(lib_handle_2_v1);
    if (lib_handle_2_v2) dlclose(lib_handle_2_v2);
}


static void switch_implementations() {
    
    current_impl_1 = (current_impl_1 == 1) ? 2 : 1;
    void *target_handle_1 = (current_impl_1 == 1) ? lib_handle_1_v1 : lib_handle_1_v2;
    current_convert = (ConvertFunc)dlsym(target_handle_1, "convert");

    current_impl_2 = (current_impl_2 == 1) ? 2 : 1;
    void *target_handle_2 = (current_impl_2 == 1) ? lib_handle_2_v1 : lib_handle_2_v2;
    current_sort = (SortFunc)dlsym(target_handle_2, "sort");
    
    if (!current_convert || !current_sort) {
        char err_msg[MAX_OUTPUT_BUFFER];
        snprintf(err_msg, sizeof(err_msg), "Ошибка переключения: не удалось найти символ.\n");
        write(STDERR_FILENO, err_msg, strlen(err_msg));
        return;
    }

    custom_write(STDOUT_FILENO, "--- РЕАЛИЗАЦИИ ПЕРЕКЛЮЧЕНЫ ---\n");
    
    char output[MAX_OUTPUT_BUFFER];
    snprintf(output, sizeof(output), "Функция 1: %s\n", (current_impl_1 == 1) ? "Двоичная (V1)" : "Троичная (V2)");
    write(STDOUT_FILENO, output, strlen(output));
    
    snprintf(output, sizeof(output), "Функция 2: %s\n", (current_impl_2 == 1) ? "Пузырьковая (V1)" : "Хоара (V2)");
    write(STDOUT_FILENO, output, strlen(output));
}

static void handle_function_1(const char *arg_str) {
    if (!current_convert) {
        custom_write(STDOUT_FILENO, "Ошибка: Функция 1 не загружена.\n");
        return;
    }

    int x;
    if (sscanf(arg_str, "%d", &x) != 1) {
        custom_write(STDOUT_FILENO, "Ошибка ввода: требуется целое число для функции 1.\n");
        return;
    }

    char *result = current_convert(x); 
    
    if (result) {
        char output[MAX_OUTPUT_BUFFER];
        snprintf(output, sizeof(output), "Результат (%s): %s\n", 
            (current_impl_1 == 1) ? "Двоичный" : "Троичный", result);
        write(STDOUT_FILENO, output, strlen(output));
        free(result); 
    } else {
        custom_write(STDOUT_FILENO, "Ошибка выделения памяти.\n");
    }
}

static void handle_function_2(const char *arg_str) {
    if (!current_sort) {
        custom_write(STDOUT_FILENO, "Ошибка: Функция 2 не загружена.\n");
        return;
    }

    int temp_array[100];
    size_t count = 0;
    const char *p = arg_str;

    while (sscanf(p, "%d", &temp_array[count]) == 1) {
        count++;
        while (*p && !isspace(*p)) p++;
        while (*p && isspace(*p)) p++;
        if (count >= 100) break;
    }

    if (count == 0) {
        custom_write(STDOUT_FILENO, "Ошибка ввода: требуется список целых чисел для функции 2.\n");
        return;
    }

    int *array_to_sort = (int*)malloc(count * sizeof(int));
    if (!array_to_sort) {
        custom_write(STDOUT_FILENO, "Ошибка выделения памяти.\n");
        return;
    }
    memcpy(array_to_sort, temp_array, count * sizeof(int));
    
    char output[MAX_OUTPUT_BUFFER];
    size_t offset = 0;
    
    offset += snprintf(output + offset, sizeof(output) - offset, "Исходный массив: ");
    for(size_t i = 0; i < count; i++) {
        offset += snprintf(output + offset, sizeof(output) - offset, "%d ", temp_array[i]);
    }
    offset += snprintf(output + offset, sizeof(output) - offset, "\n");
    write(STDOUT_FILENO, output, strlen(output));

    int *sorted_array = current_sort(array_to_sort, count); 
    
    offset = 0;
    offset += snprintf(output + offset, sizeof(output) - offset, "Результат (%s): ", 
        (current_impl_2 == 1) ? "Пузырьковая" : "Хоара");
    for(size_t i = 0; i < count; i++) {
        offset += snprintf(output + offset, sizeof(output) - offset, "%d ", sorted_array[i]);
    }
    offset += snprintf(output + offset, sizeof(output) - offset, "\n");
    write(STDOUT_FILENO, output, strlen(output));

    free(array_to_sort);
}

int main() {
    if (!load_libraries()) {
        custom_write(STDERR_FILENO, "Критическая ошибка инициализации. Выход.\n");
        unload_libraries();
        return EXIT_FAILURE;
    }

    char line[512];
    
    custom_write(STDOUT_FILENO, "--- Программа 2: Динамическая загрузка ---\n");
    custom_write(STDOUT_FILENO, "Начальные реализации: Ф1 - Двоичная, Ф2 - Пузырьковая.\n");
    custom_write(STDOUT_FILENO, "Введите команду (0 - переключить, 1 [число], 2 [массив чисел], q - выход):\n");

    while (custom_read_line(line, sizeof(line))) {
        if (line[0] == 'q' || line[0] == 'Q') break;
        
        int cmd = line[0] - '0';
        char *args = line + 1;
        while (*args == ' ' || *args == '\t') args++; 
        
        switch (cmd) {
            case 0:
                switch_implementations();
                break;
            case 1:
                handle_function_1(args);
                break;
            case 2:
                handle_function_2(args);
                break;
            default:
                custom_write(STDOUT_FILENO, "Неверная команда. Используйте 0, 1 или 2.\n");
                break;
        }
    }

    unload_libraries();
    return 0;
}