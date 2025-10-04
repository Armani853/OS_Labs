#include "os_utils.h"
#include <stdio.h>   
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
#define MAX_FILENAME_LENGTH 256
#define FILTER_LENGTH 10

void start_child_process(int read_fd, const char* child_program, const char* output_name) {
    if (dup2(read_fd, STDIN_FILENO) == -1) {
        perror("Ошибка dup2");
        exit(IO_ERROR);
    }
    close(read_fd);
    execlp(child_program, child_program, (char* )output_name, (char* )NULL);
    perror("Ошибка execlp");
    exit(EXEC_ERROR);
}

int main() {
    int pipe1_fd[2];
    int pipe2_fd[2];
    pid_t pid_1, pid_2;
    char file1_name[MAX_FILENAME_LENGTH];
    char file2_name[MAX_FILENAME_LENGTH];
    char temp_buffer[MAX_LINE_LENGTH];

    StatusCode status = STATUS_OK;
    const char* str1 = "Введите имя файла для child 1: ";    
    write(STDOUT_FILENO, "Введите имя файла для child 1: ", strlen(str1));
    fflush(stdout);
    
    ssize_t bytes_read = read(STDIN_FILENO, file1_name, MAX_FILENAME_LENGTH - 1);
    if (bytes_read <= 0) {
        return INVALID_INPUT;
    }
    if (file1_name[bytes_read - 1] == '\n') {
        file1_name[bytes_read - 1] = '\0';
    } else {
        file1_name[bytes_read] = '\0';
    }
    const char* str2 = "Введите имя файла для child 2: ";
    write(STDOUT_FILENO, "Введите имя файла для child 2: \n", strlen(str2));
    fflush(stdout);
    
    bytes_read = read(STDIN_FILENO, file2_name, MAX_FILENAME_LENGTH - 1);
    if (bytes_read <= 0) {
        return INVALID_INPUT;
    }
    if (file2_name[bytes_read - 1] == '\n') {
        file2_name[bytes_read - 1] = '\0';
    } else {
        file2_name[bytes_read] = '\0';
    }
    
    if (pipe(pipe1_fd) == -1 || pipe(pipe2_fd) == -1) {
        perror("Ошибка pipe");
        return PIPE_ERROR;
    }
    
    pid_1 = fork();
    if (pid_1 == -1) {
        perror("Ошибка fork 1");
        status = FORK_ERROR;
        goto cleanup_pipes;
    }

    if (pid_1 == 0) {
        close(pipe1_fd[1]);
        close(pipe2_fd[0]);
        close(pipe2_fd[1]);
        start_child_process(pipe1_fd[0], "./lab1/child_run", file1_name);
    }

    pid_2 = fork();
    if (pid_2 == -1) {
        perror("Ошибка fork 2");
        status = FORK_ERROR;
        goto cleanup_pipes;
    }

    if (pid_2 == 0) {
        close(pipe2_fd[1]);
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        start_child_process(pipe2_fd[0], "./lab1/child_run", file2_name);
    }

    close(pipe1_fd[0]);
    close(pipe2_fd[0]);
    const char* par_start = "Родительский процесс начался \n";
    write(STDOUT_FILENO, "Родительский процесс начался \n", strlen(par_start));
    const char* input_str = "Введите строчки \n";
    write(STDOUT_FILENO, "Введите строчки \n", strlen(input_str));
    fflush(stdout);

    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        if (strcmp(buffer, "QUIT") == 0) {
            break;
        }
        int write_fd = -1;
        
        if (len > FILTER_LENGTH) {
            write_fd = pipe2_fd[1];
            int n = snprintf(temp_buffer, sizeof(temp_buffer), "Длина Child 2: %d\n", len);
            write(STDOUT_FILENO, temp_buffer, n);
        } else {
            write_fd = pipe1_fd[1];
            int n = snprintf(temp_buffer, sizeof(temp_buffer), "Длина Child 1: %d\n", len);
            write(STDOUT_FILENO, temp_buffer, n);
        }
        fflush(stdout);
        
        if (len < sizeof(buffer) - 1) {
            buffer[len] = '\n';
            buffer[len + 1] = '\0';
            len++;
        }
        
        if (write(write_fd, buffer, len) != len) {
            perror("Ошибка с pipe");
            return PIPE_ERROR;
        }
    }

    close (pipe1_fd[1]);
    close(pipe2_fd[1]);
    waitpid(pid_1, NULL, 0);
    waitpid(pid_2, NULL, 0);
    const char* end_msg = "Родительский и детский процесс успешно завершены\n";
    write(STDOUT_FILENO, "Родительский и детский процесс успешно завершены\n", strlen(end_msg));
    fflush(stdout);
    
    return STATUS_OK;

cleanup_pipes:
    close(pipe1_fd[0]); close(pipe1_fd[1]);
    close(pipe2_fd[0]); close(pipe2_fd[1]);
    return status;
}