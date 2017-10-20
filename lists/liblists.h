#ifndef MODULE_H_INCLUDED
#define MODULE_H_INCLUDED

typedef struct Node {
    char *str;
    struct Node *next;
} List;

void init(List **a);

int is_memory_error(void *s, const char *msg);

int insert(List **a, const char *src);

void delete_list(List *a);

int str_in_list(const List *a, const char *src);

int str_input(char **str);

void input_list(List **a);

void print_list(List *a);

#endif
