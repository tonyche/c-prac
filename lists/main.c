#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liblists.h"

void 
form_xor_list(List **dest, const List *src_1, const List *src_2) 
{
    List *tmp_1 = (List *) src_1;
    List *tmp_1_second = tmp_1;
    List *tmp_2 = (List *) src_2;
    while (tmp_1 != NULL) {
        if (!str_in_list(tmp_2, tmp_1->str)) {
            insert(&(*dest), tmp_1->str);
        }
        tmp_1 = tmp_1->next;
    }
    tmp_1 = tmp_1_second;
    while (tmp_2 != NULL) {
        if (!str_in_list(tmp_1, tmp_2->str)) {
            insert(&(*dest), tmp_2->str);
        }
        tmp_2 = tmp_2->next;
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
