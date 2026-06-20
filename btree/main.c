#include <stdio.h>
#include <stdlib.h>

#include "btree.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Btree* btree = init_btree(pm);
    for(int i = 1;i <= 150; i++){
        printf("============ %d\n", i);
        insert(btree, i, NULL, NULL);
    }
    Page* root_page = get_root_page(pm);

    lseek(pm->fd, 2148, SEEK_SET);
    Page* left_page = get_page(pm->fd);
    lseek(pm->fd, 4196, SEEK_SET);
    Page* right_page = get_page(pm->fd);
    lseek(pm->fd, 6244, SEEK_SET);
    Page* right1_page = get_page(pm->fd);

        printf("============-=============================================================================\n");
    print_page(root_page);
        printf("============\n");

    print_page(left_page);
        printf("============\n");

    print_page(right_page);
        printf("============\n");

    print_page(right1_page);

    return 0;
}