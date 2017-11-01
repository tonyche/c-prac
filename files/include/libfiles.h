#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED 

enum {
    STR_BUF_SIZE = 1024,
    SEARCH_MAX = 0,
    SEARCH_MIN = -1,
    FILE_ERR = -1,
    MEMORY_ERR = -2
};

size_t search_string(size_t *length, FILE *f, int mode, 
                     void (*compar)(size_t, size_t*, size_t*, size_t), int *status);

void process_shift(size_t left, size_t right, size_t shift, FILE *f);

void fprntline(FILE *f, size_t line_pos, size_t length);

#endif