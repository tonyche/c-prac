#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libfiles.h"

void less_than(size_t cur_len, size_t *res_len, size_t *str, size_t cur_str) {
    if (cur_len < *res_len) {
        *res_len = cur_len;
        *str = cur_str;
    }
}

void more_than(size_t cur_len, size_t *res_len, size_t *str, size_t cur_str) {
    if (cur_len > *res_len) {
        *res_len = cur_len;
        *str = cur_str;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong format!\nUsage: %s <path-to-file>\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r+");
    if (f == NULL) {
        perror("Error occured while open your file");
        return -1;
    }
    size_t len_longest, len_shortest;
    int status;
    size_t pos_longest = search_string(&len_longest, f, SEARCH_MAX, more_than, &status);
    size_t pos_shortest = search_string(&len_shortest, f, SEARCH_MIN, less_than, &status); 
    if (status == MEMORY_ERR) {
        fclose(f);
        return -1;
    }
    size_t left, right, len_l, len_r;
    if (pos_longest > pos_shortest) {
        left = pos_shortest; right = pos_longest;
        len_l = len_shortest; len_r = len_longest;
    } else {
        left = pos_longest; right = pos_shortest;
        len_l = len_longest; len_r = len_shortest;
    }
    fprntline(f, pos_longest, len_longest);
    process_shift(left, right + len_r, len_l, f);
    process_shift(left, right + len_r - len_l, right - left - len_l, f);
    fclose(f);
    return 0;
}
