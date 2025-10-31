#include "../include/functions.h"

void remove_vowels(char* str) {
    if (!str) {
        return;
    }
    int write_idx = 0;
    for (int read_idx = 0; str[read_idx]; read_idx++) {
        char c = str[read_idx];
        char lower_c = tolower((unsigned char)c);
        if (lower_c != 'a' && lower_c != 'e' && lower_c != 'i' && 
            lower_c != 'o' && lower_c != 'u' && lower_c != 'y') {
                str[write_idx++] = lower_c;
            }
    }
    str[write_idx] = '\0';
}

void generate_unique_name(char* buffer, size_t buf_size, const char* prefix) {
    pid_t pid = getpid();
    unsigned long cur_time = (unsigned long)time(NULL);
    snprintf(buffer, buf_size, "/%s_%d_%lu", prefix, pid, cur_time);
}