#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "page.h"
#include "pagemanager.h"

Page* get_page(int fd){
    uint8_t page_buf[PAGE_SIZE];

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
    if(page_header->page_type & LEAF_PAGE)
        memcpy(page->children, page_buf + PAGE_HEADER_SIZE + sizeof(page->keys), sizeof(page->children));
    else
        memcpy(page->data, page_buf + PAGE_HEADER_SIZE + sizeof(page->keys), sizeof(page->data));
        
    page->header = page_header;

    return page;

}

Page* get_root_page(PageManager* pm){
    // skip the page manager header and points to the root page's first byte.
    if (lseek(pm->fd, DB_HEADER_SIZE, SEEK_SET) == -1) {
        perror("lseek");
        return NULL;
    }

    return get_page(pm->fd);
}

off_t create_new_page(PageManager* pm, int* keys, int* values, unsigned int page_type){
    Page* page = malloc(sizeof(Page));
    PageHeader* page_header = malloc(sizeof(PageHeader));
    int num_of_keys = 0;
    while(num_of_keys < MAX_KEYS_PER_PAGE && keys[num_of_keys] != 0){
        num_of_keys++;
    }
    page_header->page_type = page_type;
    page->header->num_of_keys = num_of_keys;

    page->header = page_header;

    memcpy(page->keys, keys, (MAX_KEYS_PER_PAGE + 1) * sizeof(int));
    
    if(page_type & LEAF_PAGE){
        memcpy(page->data, values, (MAX_KEYS_PER_PAGE + 2) * sizeof(int));
    }else{
        memcpy(page->children, values, (MAX_KEYS_PER_PAGE + 2) * sizeof(int));
    }

    int fd = pm->fd;
    off_t pos = lseek(fd, 0, SEEK_END);
    uint8_t page_buf[PAGE_SIZE];
    memset(page_buf, 0, PAGE_SIZE);
    memcpy(page_buf, page->header, sizeof(PageHeader));
    memcpy(page_buf + PAGE_HEADER_SIZE, page->keys, sizeof(page->keys));
    memcpy(page_buf + PAGE_HEADER_SIZE + sizeof(page->keys), page->children, sizeof(page->children));

    int written = write(fd, page_buf, PAGE_SIZE);
    if(written == -1){
        perror("write page failed");
        return -1;
    }

    return pos;
}

void insert_key_in_internal_page(Page* page, int key, int value){
    int pos = page->header->num_of_keys;

    while(pos >= 0 && (!page->keys[pos] || page->keys[pos] > key)){
        page->keys[pos + 1] = page->keys[pos];
        page->keys[pos] = key;

        page->children[pos + 2] = page->children[pos + 1];

        pos--;
    }

    page->header->num_of_keys++;

    return;
}

void insert_key_in_leaf_page(Page* page, int key, int value){
    int pos = page->header->num_of_keys;

    while(pos >= 0 && (!page->keys[pos] || page->keys[pos] > key)){
        page->keys[pos + 1] = page->keys[pos];
        page->keys[pos] = key;

        page->data[pos + 1] = page->data[pos];
        page->data[pos] = value;

        pos--;
    }

    page->header->num_of_keys++;

    return;
}

void delete_key_from_page(Page* page, int key){
    int i = 0;
    int num_of_Keys = page->header->num_of_keys;
    int pos = -1;
    while(i < num_of_Keys){
        if(page->keys[i] == key){
            pos = i;
            break;
        }
        i++;
    }
    if(pos == -1) return;

    while(pos < num_of_Keys){
        page->keys[pos] = page->keys[pos + 1];
        if(page->header->page_type & LEAF_PAGE)
            page->data[pos] = page->data[pos + 1];
        else 
            page->children[pos + 1] = page->children[pos + 2];
        pos++;
    }
    page->header->num_of_keys--;
    return;
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
    if(page->header->page_type & LEAF_PAGE){
        printf("\t Data: \n");
        for(int i = 0; i < MAX_KEYS_PER_PAGE; i++){
            printf("%d, ", page->data[i]);
        }
    }else{
        printf("\t Children: \n");
        for(int i = 0; i < MAX_KEYS_PER_PAGE + 2; i++){
            printf("%d, ", page->children[i]);
        }
    }
    printf("\n");
}