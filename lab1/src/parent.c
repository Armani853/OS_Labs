#include "os_utils.h"

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

    StatusCode status = STATUS_OK;
    printf("Введите имя файла для child 1: ");
    if (scanf("%255s", file1_name) != 1) {
        return INVALID_INPUT;
    }
    printf("Введите имя файла для child 2: ");
    if (scanf("%255s", file2_name) != 1) {
        return INVALID_INPUT;
    }
    while (getchar() != '\n');

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
        start_child_process(pipe2_fd[0], "./lab1/child_run",  file2_name);
    }

    close(pipe1_fd[0]);
    close(pipe2_fd[0]);

    printf("Родительский процесс начался \n");
    printf("Введите строчки \n");

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
            printf("Длина Child 2: %d\n", len);
        } else {
            write_fd = pipe1_fd[1];
            printf("Длина Child 1: %d\n", len);
        }
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
    printf("Родительский и детский процесс успешно завершены");
    return STATUS_OK;

cleanup_pipes:
    close(pipe1_fd[0]); close(pipe1_fd[1]);
    close(pipe2_fd[0]); close(pipe2_fd[1]);
    return status;

}