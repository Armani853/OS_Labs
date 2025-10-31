#ifndef STATUS_CODES_H
#define STATUS_CODES_H

typedef enum {
    STATUS_OK = 0,
    INVALID_INPUT = 1,
    OPEN_ERROR = 2,
    FORK_ERROR = 3,
    PIPE_ERROR = 4,
    EXEC_ERROR = 5,
    SEM_ERROR = 6,
    INIT_ERROR = 7,
    MEMORY_ERROR = 8
} StatusCode;

#endif