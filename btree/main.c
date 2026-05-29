#include <stdio.h>
#include <stdlib.h>

#include "pagemanager.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Page* root_page = get_root_page(pm);

    insert_key_in_page(root_page, 1, 1);
    insert_key_in_page(root_page, 2, 1);
    insert_key_in_page(root_page, 3, 1);
    insert_key_in_page(root_page, 100, 1);
    write_page(pm, root_page, 100);
    root_page = get_root_page(pm);
    insert_key_in_page(root_page, 101, 1);
    insert_key_in_page(root_page, 102, 1);
    insert_key_in_page(root_page, 103, 1);

    print_page(root_page);

    delete_key_from_page(root_page, 1);
    write_page(pm, root_page, 100);
    root_page = get_root_page(pm);
    print_page(root_page);

    return 0;
}