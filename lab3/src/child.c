#include "../include/functions.h"

typedef struct {
    char shm_name[64];
    char sem_name[64];
    char output_file[MAX_FILENAME_LENGTH];
} ChildArgs;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        return INVALID_INPUT;
    }

    const char* output_name = argv[1];
    const char* shm_name = argv[2];
    const char* sem_name = argv[3];
    FILE* output = NULL;
    int shm_fd = -1;
    char* shm_ptr = NULL;
    sem_t* sem_ptr = SEM_FAILED;
    char buffer[MAX_LINE_LENGTH];

    output = fopen(output_name, "w");
    if (output == NULL) {
        perror("Child: Ошибка fopen");
        return OPEN_ERROR;
    }

    sem_ptr = sem_open(sem_name, 0); 
    if (sem_ptr == SEM_FAILED) {
        perror("Child: Ошибка sem_open");
        goto cleanup_file;
    }

    shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Child: Ошибка shm_open");
        goto cleanup_sem;
    }

    shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    close(shm_fd); 
    if (shm_ptr == MAP_FAILED) {
        perror("Child: Ошибка mmap");
        goto cleanup_sem;
    }

    while (1) {
        if (sem_wait(sem_ptr) == -1) {
            perror("Child: Ошибка sem_wait");
            break;
        }
        
        if (strncmp(shm_ptr, "QUIT", 4) == 0 || shm_ptr[0] == '\0') { 
            break; 
        }

        strncpy(buffer, shm_ptr, MAX_LINE_LENGTH);
        buffer[MAX_LINE_LENGTH - 1] = '\0';
        

        remove_vowels(buffer);
        
        if (fputs(buffer, output) == EOF) {
            perror("Child: Ошибка fputs");
            break;
        }
    }

    munmap(shm_ptr, SHM_SIZE);
cleanup_sem:
    sem_close(sem_ptr);
cleanup_file:
    fclose(output);
    return STATUS_OK;
}