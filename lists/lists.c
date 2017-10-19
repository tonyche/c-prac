#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *str;
    struct Node *next;
} List;

void init(List **a) {
    *a = NULL;
}

int 
is_memory_error(void *s, const char *msg) 
{ 
    if (s == NULL) {
        fprintf(stderr, "Memory allocation error in %s!\nDetail: ", msg);
        perror(NULL);
    }
    return !!!s;
}

int 
insert(List **a, const char *src) 
{
    List *p = *a;
    List *new_node = malloc(sizeof(**a));
    if (is_memory_error(new_node, "insert()")) {
        return 0;
    }
    new_node->next = NULL;
    new_node->str = malloc((strlen(src) + 1) * sizeof(src[0]));
    if (is_memory_error(new_node, "insert()")) {
        return 0;
    }
    int i = 0;
    for (; src[i] != '\0'; i++) {
        new_node->str[i] = src[i]; 
    }
    new_node->str[i] = '\0';
    if (*a == NULL) {
        *a = new_node;
        return 1;
    }
    while ((p->next) != NULL) {
        p = p->next;
    }
    p->next = new_node;
    return 1;
}

void 
delete_list(List *a) 
{
    if (a == NULL) {
        return;
    }
    List *p = a->next;
    while (p != NULL) {
        free(a->str);
        free(a);
        a = p;
        p = p->next;
    } 
    free(a->str);
    free(a);
}

//0 if not find, 1 otherwise
int 
str_in_list(List *a, const char *src) 
{
    if (a == NULL) {
        return 0;
    }
    do {
        if (!strcmp(a->str, src)) {
            return 1;
        }
    } while ((a = a->next) != NULL);
    return 0;
}

enum { ERR_CODE_INPUT = -1 };
enum { DEFAULT_BUF_SIZE = 80 };

int 
str_input(char **str) 
{
    char c, *tmp = *str;
    size_t bufsize = DEFAULT_BUF_SIZE;
    tmp = malloc(bufsize * sizeof(tmp[0]));
    if (is_memory_error(tmp, "str_input()")) {
        return ERR_CODE_INPUT;
    }
    size_t i = 0;
    while ((c = getchar()) != EOF && c != '\n' && c != ' ') { 
        if (i == bufsize) {
            bufsize *= 2;
            char *new_tmp = realloc(tmp, bufsize * sizeof(tmp[0]));
            if (is_memory_error(new_tmp, "str_input()")) {
                free(tmp);
                return ERR_CODE_INPUT;
            }
            tmp = new_tmp;
        }
        tmp[i++] = c;
    }
    tmp[i] = '\0';
    *str = tmp;
    return !!i;
}

void 
input_list(List **a) 
{
    char *s = NULL;
    int not_null_len;
    while ((not_null_len = str_input(&s)) != ERR_CODE_INPUT && not_null_len) {
            insert(&(*a), s);
            free(s);
    }
    free(s);
}

void 
form_xor_list(List **dest, List *src_1, List *src_2) 
{
    List *tmp = src_1;
    while (src_1 != NULL) {
        if (!str_in_list(src_2, src_1->str)) {
            insert(&(*dest), src_1->str);
        }
        src_1 = src_1->next;
    }
    src_1 = tmp;
    while (src_2 != NULL) {
        if (!str_in_list(src_1, src_2->str)) {
            insert(&(*dest), src_2->str);
        }
        src_2 = src_2->next;
    }
}

void 
print_list(List *a) 
{
    if (a == NULL) {
        return;
    }
    do {
        printf("%s\n", a->str);
    } while ((a = a->next) != NULL);
}

int 
main(void) 
{
    List *L1, *L2, *L3;
    init(&L1); 
    init(&L2); 
    init(&L3);
    input_list(&L1);
    input_list(&L2);
    form_xor_list(&L3, L1, L2);
    print_list(L3);
    delete_list(L1); 
    delete_list(L2); 
    delete_list(L3);
    return 0;
}
