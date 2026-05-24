#include <stdio.h>
#include <stdlib.h>

#include "pagemanager.h"

int main(){
    PageManager* pm = create_pagemanager("my.odb", true);
    Page* root_page = get_root_page(pm);

    print_page(root_page);
    return 0;
}