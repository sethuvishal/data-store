#include <stdio.h>
#include <stdlib.h>

#include "btree.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Btree* btree = init_btree(pm);
    for(int i = 1;i <= 150; i++){
        insert(btree, i, NULL, NULL);
    }
    // Page* root_page = get_root_page(pm);

    printf("=================== TESTING GET VALUES IN BTREE =======================\n");
    for(int i = 34; i < 56; i++){
        int value = find(btree, i, NULL);
        printf("Search Key %d valud is %d\n", i , value);
    }

    update(btree, 34, 100, NULL);
    int updated_value = find(btree, 34, NULL);
    printf("Checking key(%d) after updating. value is %d\n", 34, updated_value);

    delete(btree, 34, NULL);
    int deleted_value = find(btree, 34, NULL);
    printf("Checking key(%d) after deleting. value is %d\n", 34, deleted_value);

    return 0;
}