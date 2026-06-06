#include <stdio.h>
#include <stdlib.h>

#include "pagemanager.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Page* root_page = get_root_page(pm);

    insert_key_in_leaf_page(root_page, 5, 1);
    insert_key_in_leaf_page(root_page, 3, 2);
    insert_key_in_leaf_page(root_page, 2, 3);
    insert_key_in_leaf_page(root_page, 100, 112);
    write_page(pm, root_page, 100);
    root_page = get_root_page(pm);
    insert_key_in_leaf_page(root_page, 101, 1354);
    insert_key_in_leaf_page(root_page, 102, 1234234);
    insert_key_in_leaf_page(root_page, 103, 9989);

    print_page(root_page);

    delete_key_from_page(root_page, 5);
    write_page(pm, root_page, 100);
    root_page = get_root_page(pm);
    print_page(root_page);

    return 0;
}