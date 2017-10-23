#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liblists.h"

void 
form_xor_list(List **dest, const List *src_1, const List *src_2) 
{
    const List *tmp_1_second = src_1;
    while (src_1 != NULL) {
        if (!str_in_list(src_2, src_1->str)) {
            insert(&(*dest), src_1->str);
        }
        src_1 = src_1->next;
    }
    src_1 = tmp_1_second;
    while (src_2 != NULL) {
        if (!str_in_list(src_1, src_2->str)) {
            insert(&(*dest), src_2->str);
        }
        src_2 = src_2->next;
    }
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
