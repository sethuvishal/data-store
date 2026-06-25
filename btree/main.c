#include <stdio.h>
#include <stdlib.h>

#include "btree.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Btree* btree = init_btree(pm);
    for(int i = 1;i <= 140; i++){
        insert(btree, i, NULL, NULL);
    }

    printf("=================== TESTING GET VALUES IN BTREE =======================\n");
    for(int i = 34; i < 56; i++){
        find(btree, i, NULL);
        // printf("Search Key %d valud is %d\n", i , value);
    }

    update(btree, 34, 100, NULL);
    int updated_value = find(btree, 34, NULL);
    printf("Checking key(%d) after updating. value is %d\n", 34, updated_value);

    for(int i = 34; i <= 50; i++){
        delete(btree, i, NULL, NULL);
        int deleted_value = find(btree, i, NULL);
        printf("Checking key(%d) after deleting. value is %d\n", i, deleted_value);
    }
    for(int i = 117; i <= 140; i++){
        delete(btree, i, NULL, NULL);
        int deleted_value = find(btree, i, NULL);
        printf("Checking key(%d) after deleting. value is %d\n", i, deleted_value);
    }

    // printing pages after deleting keys
    Page* root_page = get_root_page(pm);
    print_page(root_page);

    // for(int i = 0; i < root_page->header->num_of_keys + 1; i++){
    //     Page* page = get_page(pm->fd, root_page->children[i]);
    //     print_page(page);
    // }

    return 0;
}