#include <stdio.h>
#include <stdlib.h>

#include "btree.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Btree* btree = init_btree(pm);
    for(int i = 1;i <= 101; i++){
        insert(btree, i, NULL);
    }
    Page* root_page = get_root_page(pm);

    print_page(root_page);

    return 0;
}