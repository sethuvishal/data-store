#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "page.h"
#include "pagemanager.h"

Page* get_page(int fd){
    char page_buf[PAGE_SIZE];

    ssize_t n = read(fd, page_buf, PAGE_SIZE);

    if (n < 0) {
        perror("read");
        return NULL;
    }
    PageHeader* page_header = malloc(sizeof(PageHeader));
    if(page_header == NULL){
        perror("malloc");
        return NULL;
    }
    memcpy(page_header, page_buf, sizeof(PageHeader));

    Page* page = malloc(sizeof(Page));
    if(page == NULL){
        perror("malloc");
        return NULL;
    }

    memcpy(page->keys, page_buf + PAGE_HEADER_SIZE, sizeof(page->keys));
    memcpy(page->children, page_buf + PAGE_HEADER_SIZE + sizeof(page->keys), sizeof(page->children));
    page->header = page_header;

    return page;

}

Page* get_root_page(PageManager* pm){
    // skip the page manager header and points to the root page's first byte.
    if (lseek(pm->fd, PAGE_MANAGER_HEADER_SIZE, SEEK_SET) == -1) {
        perror("lseek");
        return NULL;
    }

    return get_page(pm->fd);
}

void print_page(Page* page){
    printf("Page Header: \n");
    printf("\tPage type: %d\n", page->header->page_type);
    printf("\t Number of keys: %d\n", page->header->num_of_keys);

    printf("Page: \n");
    printf("\t Keys: \n");
    for(int i = 0; i < MAX_KEYS_PER_PAGE + 1; i++){
        printf("%d, ", page->keys[i]);
    }
    printf("\n");
    printf("\t Children: \n");
    for(int i = 0; i < MAX_KEYS_PER_PAGE + 2; i++){
        printf("%d, ", page->children[i]);
    }
}