#include <stdio.h>
#include <math.h>

#include "btree.h"

Btree* init_btree(PageManager* pm){
    Btree* btree = malloc(sizeof(Btree));
    btree->pm = pm;

    return btree;
}

void insert(Btree* btree, int value, Page* page){
    if(page == NULL){
        page = get_root_page(btree->pm);
    }
    int num_of_keys = page->header->num_of_keys;

    if(page->header->page_type & LEAF_PAGE){
        page->keys[num_of_keys] = num_of_keys == 0 ? 1 : page->keys[num_of_keys - 1] + 1;
        page->data[num_of_keys] = value;
        page->header->num_of_keys++;
        if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
            split_node(btree, page);
        }else write_page(btree->pm, page, 100);
    }else{
        // insert(btree, value, page->children[num_of_keys]);
    }
}

void split_node(Btree* btree, Page* page){
    int *values, *keys;
    if(page->header->page_type & LEAF_PAGE){
        values = page->data;
    }else{
        values = page->children;
    }
    keys = page->keys;

    int n = (int)ceil(MAX_KEYS_PER_PAGE / 2) - 1;
    int new_left_node_values[MAX_KEYS_PER_PAGE + 2] = {0};
    memcpy(new_left_node_values, values, (n) * sizeof(int));
    int new_right_node_values[MAX_KEYS_PER_PAGE + 2] = {0};
    memcpy(new_right_node_values, values + n, (n + 1) * sizeof(int));

    int new_left_node_keys[MAX_KEYS_PER_PAGE + 1] = {0};
    memcpy(new_left_node_keys, keys, (n) * sizeof(int));
    int new_right_node_keys[MAX_KEYS_PER_PAGE + 1] = {0};
    memcpy(new_right_node_keys, keys + n, (n + 1) * sizeof(int));

    unsigned int  page_type = page->header->page_type & LEAF_PAGE ? LEAF_PAGE : NON_LEAF_PAGE; 
    off_t left_node_pointer = create_new_page(btree->pm, new_left_node_keys, new_left_node_values, page_type);
    off_t right_node_pointer = create_new_page(btree->pm, new_right_node_keys, new_right_node_values, page_type);
    
    int new_keys[MAX_KEYS_PER_PAGE + 1]= {new_right_node_keys[0]};
    int new_values[MAX_KEYS_PER_PAGE + 2] = {left_node_pointer, right_node_pointer};
    page->header->num_of_keys = 1;
    page->header->page_type &= ~LEAF_PAGE;
    memcpy(page->keys, new_keys, (MAX_KEYS_PER_PAGE + 1) * sizeof(int));
    memcpy(page->children, new_values, (MAX_KEYS_PER_PAGE + 2) * sizeof(int));
    write_page(btree->pm, page, 100);
}