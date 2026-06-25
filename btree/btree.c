#include <stdio.h>
#include <math.h>

#include "btree.h"

Btree* init_btree(PageManager* pm){
    Btree* btree = malloc(sizeof(Btree));
    btree->pm = pm;

    return btree;
}

void insert(Btree* btree, int value, Page* page, Page* parent_page){
    if(page == NULL){
        page = get_root_page(btree->pm);
    }
    int num_of_keys = page->header->num_of_keys;
    if(page->header->page_type & LEAF_PAGE){
        page->keys[num_of_keys] = num_of_keys == 0 ? 1 : page->keys[num_of_keys - 1] + 1;
        page->data[num_of_keys] = value;
        page->header->num_of_keys++;
        if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
            split_node(btree, page, parent_page);
        }else{
            write_page(btree->pm, page, page->header->pos);
        } 
    }else{
        int fd = btree->pm->fd;
        Page* next_page = get_page(fd, page->children[num_of_keys]);
        insert(btree, value, next_page, page);
        if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
            split_node(btree, page, parent_page);
        }
    }
    if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
        split_node(btree, page, parent_page);
    }
}

void split_node(Btree* btree, Page* page, Page* parent_page){
    int *values, *keys;
    if(page->header->page_type & LEAF_PAGE){
        values = page->data;
    }else{
        values = page->children;
    }
    keys = page->keys;

    int n = (int)ceil(MAX_KEYS_PER_PAGE / 2);
    int new_left_node_values[MAX_KEYS_PER_PAGE + 2] = {0};
    memcpy(new_left_node_values, values, (n) * sizeof(int));
    int new_right_node_values[MAX_KEYS_PER_PAGE + 2] = {0};
    memcpy(new_right_node_values, values + n, (n + 1) * sizeof(int));

    int new_left_node_keys[MAX_KEYS_PER_PAGE + 1] = {0};
    memcpy(new_left_node_keys, keys, (n) * sizeof(int));
    int new_right_node_keys[MAX_KEYS_PER_PAGE + 1] = {0};
    memcpy(new_right_node_keys, keys + n, (n + 1) * sizeof(int));
    unsigned int  page_type = page->header->page_type & LEAF_PAGE ? LEAF_PAGE : NON_LEAF_PAGE; 
    if(parent_page == NULL){
        // if not parent page then the LEAF_PAGE and ROOT_PAGE is same
        // in this case we are spliting the nodes by half and add the middle key in the root page 
        // rest in the LEFT and RIGHT node of that key
        off_t left_node_pointer = create_new_page(btree->pm, new_left_node_keys, new_left_node_values, page_type);
        off_t right_node_pointer = create_new_page(btree->pm, new_right_node_keys, new_right_node_values, page_type);
        
        int new_keys[MAX_KEYS_PER_PAGE + 1]= {new_right_node_keys[0]};
        int new_values[MAX_KEYS_PER_PAGE + 2] = {left_node_pointer, right_node_pointer};
        page->header->num_of_keys = 1;
        page->header->page_type &= ~LEAF_PAGE;
        memcpy(page->keys, new_keys, (MAX_KEYS_PER_PAGE + 1) * sizeof(int));
        memcpy(page->children, new_values, (MAX_KEYS_PER_PAGE + 2) * sizeof(int));
        write_page(btree->pm, page, page->header->pos);
    }else{
        // if parent_page is not null then we have to add a new key to the parent page
        off_t right_node_pointer = create_new_page(btree->pm, new_right_node_keys, new_right_node_values, page_type);
        memcpy(page->keys, keys, (n) * sizeof(int));
        if(page_type & LEAF_PAGE){
            memcpy(page->data, values, (n) * sizeof(int));
        }else memcpy(page->children, values, (n) * sizeof(int));
        page->header->num_of_keys = n;
        write_page(btree->pm, page, page->header->pos);
        insert_key_in_internal_page(parent_page, new_right_node_keys[0], right_node_pointer);
        write_page(btree->pm, parent_page, parent_page->header->pos);
    }
}

int find(Btree* btree, int search_key, Page* page){
    if(page == NULL){
        page = get_root_page(btree->pm);
    }

    unsigned int page_type = page->header->page_type;
    if(page_type & LEAF_PAGE){
        for(int i = 0; i < page->header->num_of_keys; i++){
            if(page->keys[i] > search_key) break;
            if(page->keys[i] == search_key) return page->data[i];
        }
    }else{
        for(int i = 0 ; i < page->header->num_of_keys; i++){
            if(page->keys[i] > search_key){
                Page* next_page = get_page(btree->pm->fd, page->children[i]);
                return find(btree, search_key, next_page);
                break;
            }
        }
    }
    return 0;
}

bool update(Btree* btree, int update_key, int new_value, Page* page){
    if(page == NULL){
        page = get_root_page(btree->pm);
    }

    unsigned int page_type = page->header->page_type;
    if(page_type & LEAF_PAGE){
        for(int i = 0; i < page->header->num_of_keys; i++){
            if(page->keys[i] > update_key) break;
            if(page->keys[i] == update_key) {
                page->data[i] = new_value;
                write_page(btree->pm, page, page->header->pos);
                return true;
            }
        }
    }else{
        for(int i = 0 ; i < page->header->num_of_keys; i++){
            if(page->keys[i] > update_key){
                Page* next_page = get_page(btree->pm->fd, page->children[i]);
                return update(btree, update_key, new_value, next_page);
                break;
            }
        }
    }
    return false;
}


bool borrow_from_left_page(Btree* btree, Page* parent_page, Page* current_page){
    int current_page_idx = 0;
    while(current_page_idx < parent_page->header->num_of_keys){
        if(parent_page->children[current_page_idx] == current_page->header->pos) break;
        current_page_idx++;
    }

    if(current_page_idx == 0 || current_page_idx > parent_page->header->num_of_keys) return false;

    Page* left_page = get_page(btree->pm->fd, parent_page->children[current_page_idx - 1]);
    int left_page_num_of_keys = left_page->header->num_of_keys;
    if(left_page_num_of_keys <= MAX_KEYS_PER_PAGE / 2) return false;

    if(current_page->header->page_type & LEAF_PAGE){
        insert_key_in_leaf_page(current_page, left_page->keys[left_page_num_of_keys - 1], left_page->data[left_page_num_of_keys - 1]);
    }else{
        insert_key_in_internal_page(current_page, left_page->keys[left_page_num_of_keys - 1], left_page->children[left_page_num_of_keys - 1]);
    }
    parent_page->keys[current_page_idx - 1] = left_page->keys[left_page_num_of_keys - 2];
    delete_key_from_page(left_page, left_page->keys[left_page_num_of_keys - 1]);
    write_page(btree->pm, current_page, current_page->header->pos);
    write_page(btree->pm, left_page, left_page->header->pos);
    write_page(btree->pm, parent_page, parent_page->header->pos);
    return true;
}

bool borrow_from_right_page(Btree* btree, Page* parent_page, Page* current_page){
    int current_page_idx = 0;
    while(current_page_idx < parent_page->header->num_of_keys){
        if(parent_page->children[current_page_idx] == current_page->header->pos) break;
        current_page_idx++;
    }
    if(current_page_idx >= parent_page->header->num_of_keys) return false;

    Page* right_page = get_page(btree->pm->fd, parent_page->children[current_page_idx + 1]);
    int right_page_num_of_keys = right_page->header->num_of_keys;
    if(right_page_num_of_keys <= MAX_KEYS_PER_PAGE / 2) return false;

    if(current_page->header->page_type & LEAF_PAGE){
        insert_key_in_leaf_page(current_page, right_page->keys[0], right_page->data[0]);
    }else{
        insert_key_in_internal_page(current_page, right_page->keys[0], right_page->children[0]);
    }
    parent_page->keys[current_page_idx] = right_page->keys[0];
    delete_key_from_page(right_page, right_page->keys[0]);
    write_page(btree->pm, current_page, current_page->header->pos);
    write_page(btree->pm, right_page, right_page->header->pos);
    write_page(btree->pm, parent_page, parent_page->header->pos);
    return true;
}

bool delete(Btree* btree, int delete_key, Page* page, Page* parent_page){
    if(page == NULL){
        page = get_root_page(btree->pm);
    }

    unsigned int page_type = page->header->page_type;
    if(page_type & LEAF_PAGE){
        for(int i = 0; i < page->header->num_of_keys; i++){
            if(page->keys[i] > delete_key) break;
            if(page->keys[i] == delete_key) {
                delete_key_from_page(page, delete_key);
                if(page->header->num_of_keys < MAX_KEYS_PER_PAGE / 2){ // underflow borrow a key from left or right page
                    if(borrow_from_right_page(btree, parent_page, page)) return true;
                    else if(borrow_from_left_page(btree, parent_page, page)) return true;

                    // if cannot borrow a key from left and right page
                    // let the page underflow for now
                    // need to fix it later by merging the page with right page.
                }
                write_page(btree->pm, page, page->header->pos);
                return true;
            }
        }
    }else{
        for(int i = 0 ; i < page->header->num_of_keys; i++){
            if(page->keys[i] > delete_key){
                Page* next_page = get_page(btree->pm->fd, page->children[i]);
                bool deleted = delete(btree, delete_key, next_page, page);
                return deleted;
            }
        }
    }
    return false;
}