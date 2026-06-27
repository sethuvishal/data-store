#include <stdio.h>
#include <stdlib.h>

#include "btree.h"
#include "page.h"

int main(){
    PageManager* pm = pagemanager_create("my.odb", true);
    Btree* btree = btree_create(pm);
    for(int i = 1;i <= 5061; i++){
        btree_insert(btree, i, NULL, NULL);
    }

    printf("=================== TESTING GET VALUES IN BTREE =======================\n");
    for(int i = 34; i < 56; i++){
        btree_find(btree, i, NULL);
        // printf("Search Key %d valud is %d\n", i , value);
    }

    btree_update(btree, 34, 100, NULL);
    int updated_value = btree_find(btree, 34, NULL);
    printf("Checking key(%d) after updating. value is %d\n", 34, updated_value);

    // for(int i = 34; i <= 50; i++){
    //     btree_delete(btree, i, NULL, NULL);
    //     int deleted_value = btree_find(btree, i, NULL);
    //     printf("Checking key(%d) after deleting. value is %d\n", i, deleted_value);
    // }
    // for(int i = 117; i <= 137; i++){
    //     btree_delete(btree, i, NULL, NULL);
    //     int deleted_value = btree_find(btree, i, NULL);
    //     printf("Checking key(%d) after deleting. value is %d\n", i, deleted_value);
    // }
    btree_delete(btree, 137, NULL, NULL);

    // printing pages after deleting keys
    Page* root_page = page_read_root(pm);
    page_debug_print(root_page);

    if(root_page->header->page_type == ROOT_PAGE){
        for(int i = 0; i < root_page->header->num_of_keys + 1; i++){
            Page* page = page_read_at(pm->fd, root_page->children[i]);
            page_debug_print(page);
        }
    }
    Page* last_page = page_read_at(pm->fd, 206948);
    page_debug_print(last_page);

    return 0;
}
