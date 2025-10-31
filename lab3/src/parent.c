#include "../include/functions.h"

typedef struct {
    char shm_name[64];
    char sem_name[64];
    int shm_fd;
    char* shm_ptr;
    sem_t* sem_ptr;
    pid_t pid;
} Ipc_channel;

StatusCode init_ipc_channel(Ipc_channel* channel, const char* prefix) {
    generate_unique_name(channel->shm_name, sizeof(channel->shm_name), prefix);
    generate_unique_name(channel->sem_name, sizeof(channel->sem_name), prefix);

    channel->shm_fd = shm_open(channel->shm_name, O_CREAT | O_RDWR, 0666);
    if (channel->shm_fd == -1) {
        return OPEN_ERROR;
    }

    ftruncate(channel->shm_fd, SHM_SIZE);
    channel->shm_ptr = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, channel->shm_fd, 0);
    close(channel->shm_fd);
    if (channel->shm_ptr == MAP_FAILED) {
        return MEMORY_ERROR;
    }
    channel->shm_ptr[0] = '\0';
    channel->sem_ptr = sem_open(channel->sem_name, O_CREAT, 0666, 0);
    if (channel->sem_ptr == MAP_FAILED) {
        munmap(channel->shm_ptr, SHM_SIZE);
        shm_unlink(channel->shm_name);
        return SEM_ERROR;
    }
    return STATUS_OK;
}

void start_child_process(Ipc_channel* channel, const char* child_program, const char* output_name) {
    char child_path[256];
    snprintf(child_path, sizeof(child_path), "./lab3/%s", child_program);

    execlp(child_path, child_path, (char*)output_name,
           (char*)channel->shm_name, (char*)channel->sem_name, (char*)NULL);
    exit(EXEC_ERROR);
}

void cleanup_ipc(Ipc_channel* channel) {
    if (channel->shm_ptr != MAP_FAILED) {
        munmap(channel->shm_ptr, SHM_SIZE);
    }
    shm_unlink(channel->shm_name);

    if (channel->sem_ptr != SEM_FAILED) {
        sem_close(channel->sem_ptr);
        sem_unlink(channel->sem_name);
    }
}

int main() {
    Ipc_channel ch1, ch2;
    char file1_name[MAX_FILENAME_LENGTH], file2_name[MAX_FILENAME_LENGTH];
    char buffer[MAX_LINE_LENGTH];
    StatusCode status = STATUS_OK;

    const char* prompt1 = "Введите имя файла для child 1: ";
    write(STDOUT_FILENO, prompt1, strlen(prompt1));
    if (fgets(file1_name, MAX_FILENAME_LENGTH, stdin) == NULL) return INVALID_INPUT;
    file1_name[strcspn(file1_name, "\n")] = '\0';

    const char* prompt2 = "Введите имя файла для child 2: ";
    write(STDOUT_FILENO, prompt2, strlen(prompt2));
    if (fgets(file2_name, MAX_FILENAME_LENGTH, stdin) == NULL) return INVALID_INPUT;
    file2_name[strcspn(file2_name, "\n")] = '\0';

    if (init_ipc_channel(&ch1, "ch1") != STATUS_OK ||
        init_ipc_channel(&ch2, "ch2") != STATUS_OK) {
        status = INIT_ERROR;
        goto cleanup_all;
    }

    if ((ch1.pid = fork()) == -1) {
        const char* error_msg = "Parent: Ошибка fork 1\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        status = FORK_ERROR;
        goto cleanup_all;
    }
    if (ch1.pid == 0) {
        start_child_process(&ch1, "./child_run", file1_name);
    }

    if ((ch2.pid = fork()) == -1) {
        const char* error_msg = "Parent: Ошибка fork 2\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        status = FORK_ERROR;
        goto cleanup_all;
    }
    if (ch2.pid == 0) {
        start_child_process(&ch2, "./child_run", file2_name);
    }

    const char* ready_msg = "\n--- Parent: Ready for input ---\n";
    write(STDOUT_FILENO, ready_msg, strlen(ready_msg));

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        int len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
        } else {
            if (len < sizeof(buffer) - 1) {
                buffer[len++] = '\n';
                buffer[len] = '\0';
            }
        }

        if (strncmp(buffer, "QUIT", 4) == 0) {
            break;
        }

        Ipc_channel* target_channel;
        if (len - 1 > FILTER_LENGTH) {
            target_channel = &ch2;
            const char* send_msg2 = "Parent: Sending to CHILD 2 (length: ";
            write(STDOUT_FILENO, send_msg2, strlen(send_msg2));
            char len_buf[32];
            snprintf(len_buf, sizeof(len_buf), "%d", len - 1);
            write(STDOUT_FILENO, len_buf, strlen(len_buf));
            write(STDOUT_FILENO, ")\n", 2);
        } else {
            target_channel = &ch1;
            const char* send_msg1 = "Parent: Sending to CHILD 1 (length: ";
            write(STDOUT_FILENO, send_msg1, strlen(send_msg1));
            char len_buf[32];
            snprintf(len_buf, sizeof(len_buf), "%d", len - 1);
            write(STDOUT_FILENO, len_buf, strlen(len_buf));
            write(STDOUT_FILENO, ")\n", 2);
        }

        strncpy(target_channel->shm_ptr, buffer, SHM_SIZE);
        target_channel->shm_ptr[SHM_SIZE - 1] = '\0';

        if (sem_post(target_channel->sem_ptr) == -1) {
            const char* error_msg = "Parent: Ошибка sem_post\n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
            status = SEM_ERROR;
            break;
        }
    }

    strcpy(ch1.shm_ptr, "QUIT");
    sem_post(ch1.sem_ptr);

    strcpy(ch2.shm_ptr, "QUIT");
    sem_post(ch2.sem_ptr);

    waitpid(ch1.pid, NULL, 0);
    waitpid(ch2.pid, NULL, 0);
    const char* child_done_msg = "Parent: Дочерние процессы завершены.\n";
    write(STDOUT_FILENO, child_done_msg, strlen(child_done_msg));

cleanup_all:
    cleanup_ipc(&ch1);
    cleanup_ipc(&ch2);
    const char* ipc_cleanup_msg = "Parent: Все IPC-ресурсы очищены.\n";
    write(STDOUT_FILENO, ipc_cleanup_msg, strlen(ipc_cleanup_msg));
    return status;
}
