#include <stdio.h>
#include <math.h>

#include "btree.h"

// Create a B-Tree backed by the provided PageManager.
Btree* btree_create(PageManager* pm){
    Btree* tree = malloc(sizeof(Btree));
    tree->pm = pm;
    return tree;
}

// Insert a value into the tree. Keys in leaves auto-increment in this prototype.
void btree_insert(Btree* tree, int value, Page* page, Page* parent){
    if(page == NULL){
        page = page_read_root(tree->pm);
    }
    int num_of_keys = page->header->num_of_keys;
    if(page->header->page_type & LEAF_PAGE){
        page->keys[num_of_keys] = num_of_keys == 0 ? 1 : page->keys[num_of_keys - 1] + 1;
        page->data[num_of_keys] = value;
        page->header->num_of_keys++;
        if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
            btree_split_node(tree, page, parent);
        }else{
            pagemanager_write_page(tree->pm, page, page->header->pos);
        }
    }else{
        int fd = tree->pm->fd;
        Page* next_page = page_read_at(fd, page->children[num_of_keys]);
        btree_insert(tree, value, next_page, page);
        if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
            btree_split_node(tree, page, parent);
        }
    }
    if(page->header->num_of_keys == MAX_KEYS_PER_PAGE){
        btree_split_node(tree, page, parent);
    }
}

// Split a full node into two siblings and push the lowest key of the right
// sibling up into the parent. If splitting the root, the root becomes an
// internal page with two children.
void btree_split_node(Btree* tree, Page* page, Page* parent){
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
    if(parent == NULL){
        // Root split: create left/right children and turn current page into
        // an internal root with a single separator and two child pointers.
        off_t left_node_pointer = page_create_and_append(tree->pm, new_left_node_keys, new_left_node_values, page_type);
        off_t right_node_pointer = page_create_and_append(tree->pm, new_right_node_keys, new_right_node_values, page_type);
        
        int new_keys[MAX_KEYS_PER_PAGE + 1]= {new_right_node_keys[0]};
        int new_values[MAX_KEYS_PER_PAGE + 2] = {left_node_pointer, right_node_pointer};
        page->header->num_of_keys = 1;
        page->header->page_type &= ~LEAF_PAGE;
        memcpy(page->keys, new_keys, (MAX_KEYS_PER_PAGE + 1) * sizeof(int));
        memcpy(page->children, new_values, (MAX_KEYS_PER_PAGE + 2) * sizeof(int));
        pagemanager_write_page(tree->pm, page, page->header->pos);
    }else{
        // Non-root split: shrink current page to left side and append a right
        // sibling, then insert separator + pointer in parent.
        off_t right_node_pointer = page_create_and_append(tree->pm, new_right_node_keys, new_right_node_values, page_type);
        memcpy(page->keys, keys, (n) * sizeof(int));
        if(page_type & LEAF_PAGE){
            memcpy(page->data, values, (n) * sizeof(int));
        }else memcpy(page->children, values, (n) * sizeof(int));
        page->header->num_of_keys = n;
        pagemanager_write_page(tree->pm, page, page->header->pos);
        page_insert_internal(parent, new_right_node_keys[0], right_node_pointer);
        pagemanager_write_page(tree->pm, parent, parent->header->pos);
    }
}

int btree_find(Btree* tree, int search_key, Page* page){
    if(page == NULL){
        page = page_read_root(tree->pm);
    }

    unsigned int page_type = page->header->page_type;
    if(page_type & LEAF_PAGE){
        for(int i = 0; i < page->header->num_of_keys; i++){
            if(page->keys[i] > search_key) return 0;
            if(page->keys[i] == search_key) return page->data[i];
        }
        return 0;
    }else{
        for(int i = 0 ; i < page->header->num_of_keys; i++){
            if(page->keys[i] > search_key){
                Page* next_page = page_read_at(tree->pm->fd, page->children[i]);
                return btree_find(tree, search_key, next_page);
            }
        }
    }
    Page* last_page = page_read_at(tree->pm->fd, page->children[page->header->num_of_keys]);
    return btree_find(tree, search_key, last_page);
}

bool btree_update(Btree* tree, int update_key, int new_value, Page* page){
    if(page == NULL){
        page = page_read_root(tree->pm);
    }

    unsigned int page_type = page->header->page_type;
    if(page_type & LEAF_PAGE){
        for(int i = 0; i <= page->header->num_of_keys; i++){
            if(page->keys[i] > update_key) break;
            if(page->keys[i] == update_key) {
                page->data[i] = new_value;
                pagemanager_write_page(tree->pm, page, page->header->pos);
                return true;
            }
        }
    }else{
        for(int i = 0 ; i <= page->header->num_of_keys; i++){
            if(page->keys[i] > update_key){
                Page* next_page = page_read_at(tree->pm->fd, page->children[i]);
                return btree_update(tree, update_key, new_value, next_page);
            }
        }
    }
    Page* last_page = page_read_at(tree->pm->fd, page->children[page->header->num_of_keys]);
    return btree_update(tree, update_key, new_value, last_page);
}

static void merge_leaf_nodes(
    Btree* tree,
    Page *left,
    Page *right,
    Page* parent_page
){
    int separator_index = 0;
    while (separator_index <= parent_page->header->num_of_keys){
        if (parent_page->children[separator_index] == left->header->pos)
            break;
        separator_index++;
    }

    // Copy all keys from right.
    for (int i = 0; i < right->header->num_of_keys; i++){
        left->keys[left->header->num_of_keys] = right->keys[i];
        left->header->num_of_keys++;
    }

    // Copy all children.
    int left_child_start = left->header->num_of_keys - right->header->num_of_keys;
    for (int i = 0; i <= right->header->num_of_keys; i++){
        left->data[left_child_start + i] = right->data[i];
    }

    // Remove separator from parent.
    if(parent_page->header->page_type & ROOT_PAGE && parent_page->header->num_of_keys == 1){
        left->header->pos = parent_page->header->pos;
        left->header->page_type |= parent_page->header->page_type;
        pagemanager_write_page(tree->pm, left, left->header->pos);
    }else{
        if(separator_index == parent_page->header->num_of_keys){
            // just delete the last key
            unsigned int separator_index_value = parent_page->children[separator_index];
            page_delete_key(parent_page, parent_page->keys[separator_index]);
            parent_page->children[separator_index] = separator_index_value;
            pagemanager_write_page(tree->pm, parent_page, parent_page->header->pos);
        }else{
            page_delete_key(parent_page, parent_page->keys[separator_index]);
            pagemanager_write_page(tree->pm, parent_page, parent_page->header->pos);
        }
        pagemanager_write_page(tree->pm, left, left->header->pos);
    }

    free(right);
}

static void merge_internal_pages(Btree *tree, Page *left, Page *right, Page *parent_page){
    int child_index = 0;

    while (child_index <= parent_page->header->num_of_keys)
    {
        if (parent_page->children[child_index]
            == left->header->pos)
            break;

        child_index++;
    }

    int separator =
        parent_page->keys[child_index];

    // Bring separator down.
    left->keys[left->header->num_of_keys] = separator;
    left->header->num_of_keys++;

    // Copy all keys from right.
    for (int i = 0; i < right->header->num_of_keys; i++){
        left->keys[left->header->num_of_keys] = right->keys[i];
        left->header->num_of_keys++;
    }

    // Copy all children.
    int left_child_start =
        left->header->num_of_keys -
        right->header->num_of_keys;

    for (int i = 0; i <= right->header->num_of_keys; i++){
        left->children[left_child_start + i] = right->children[i];
    }

    // Remove separator from parent.
    page_delete_key(parent_page, separator);

    pagemanager_write_page(tree->pm,
               left,
               left->header->pos);

    pagemanager_write_page(tree->pm,
               parent_page,
               parent_page->header->pos);

    free(right);
}

bool btree_borrow_from_left_sibling(Btree* tree, Page* parent_page, Page* current_page){
    int current_page_idx = 0;
    while(current_page_idx <= parent_page->header->num_of_keys){
        if(parent_page->children[current_page_idx] == current_page->header->pos) break;
        current_page_idx++;
    }

    if(current_page_idx == 0 || current_page_idx > parent_page->header->num_of_keys) return false;

    Page* left_page = page_read_at(tree->pm->fd, parent_page->children[current_page_idx - 1]);
    int left_page_num_of_keys = left_page->header->num_of_keys;
    if(left_page_num_of_keys <= MAX_KEYS_PER_PAGE / 2){
        if(left_page->header->page_type & LEAF_PAGE)
            merge_leaf_nodes(tree, left_page, current_page, parent_page);
        else
            merge_internal_pages(tree, left_page, current_page, parent_page);
        return true;
    }

    if(current_page->header->page_type & LEAF_PAGE){
        page_insert_leaf(current_page, left_page->keys[left_page_num_of_keys - 1], left_page->data[left_page_num_of_keys - 1]);
    }else{
        page_insert_internal(current_page, left_page->keys[left_page_num_of_keys - 1], left_page->children[left_page_num_of_keys - 1]);
    }
    parent_page->keys[current_page_idx - 1] = current_page->keys[0];
    page_delete_key(left_page, left_page->keys[left_page_num_of_keys - 1]);
    pagemanager_write_page(tree->pm, current_page, current_page->header->pos);
    pagemanager_write_page(tree->pm, left_page, left_page->header->pos);
    pagemanager_write_page(tree->pm, parent_page, parent_page->header->pos);
    return true;
}

bool btree_borrow_from_right_sibling(Btree* tree, Page* parent_page, Page* current_page){
    int current_page_idx = 0;
    while(current_page_idx <= parent_page->header->num_of_keys){
        if(parent_page->children[current_page_idx] == current_page->header->pos) break;
        current_page_idx++;
    }
    if(current_page_idx >= parent_page->header->num_of_keys) return false;

    Page* right_page = page_read_at(tree->pm->fd, parent_page->children[current_page_idx + 1]);
    int right_page_num_of_keys = right_page->header->num_of_keys;
    if(right_page_num_of_keys <= MAX_KEYS_PER_PAGE / 2){
        if(right_page->header->page_type & LEAF_PAGE)
            merge_leaf_nodes(tree, current_page, right_page, parent_page);
        else
            merge_internal_pages(tree, current_page, right_page, parent_page);
        return true;
    }

    if(current_page->header->page_type & LEAF_PAGE){
        page_insert_leaf(current_page, right_page->keys[0], right_page->data[0]);
    }else{
        page_insert_internal(current_page, right_page->keys[0], right_page->children[0]);
    }
    page_delete_key(right_page, right_page->keys[0]);
    parent_page->keys[current_page_idx] = right_page->keys[0];
    pagemanager_write_page(tree->pm, current_page, current_page->header->pos);
    pagemanager_write_page(tree->pm, right_page, right_page->header->pos);
    pagemanager_write_page(tree->pm, parent_page, parent_page->header->pos);
    return true;
}

static void update_parent_separator(Page *parent, unsigned int child_pos, int new_first_key){
    if (parent == NULL)
        return;

    int child_idx = -1;

    for (int i = 0; i <= parent->header->num_of_keys; i++) {
        if ((unsigned int)parent->children[i] == child_pos) {
            child_idx = i;
            break;
        }
    }

    if (child_idx <= 0)
        return;

    parent->keys[child_idx - 1] = new_first_key;
}

bool btree_delete(Btree* tree, int delete_key, Page* page, Page* parent_page){
    if(page == NULL){
        page = page_read_root(tree->pm);
    }

    unsigned int page_type = page->header->page_type;
    if(page_type & LEAF_PAGE){
        int old_first = page->keys[0];
        page_delete_key(page, delete_key);
        if(parent_page != NULL && page->header->num_of_keys < MAX_KEYS_PER_PAGE / 2){
            // underflow: try to borrow a key from right or left sibling
            if(btree_borrow_from_right_sibling(tree, parent_page, page)) return true;
            else if(btree_borrow_from_left_sibling(tree, parent_page, page)) return true;
        }
        if (delete_key == old_first && page->header->num_of_keys > 0 &&  parent_page != NULL) {
            update_parent_separator(parent_page, page->header->pos, page->keys[0]);
            pagemanager_write_page(tree->pm, parent_page, parent_page->header->pos);
        }
        pagemanager_write_page(tree->pm, page, page->header->pos);
        return true;
    }else{
        for(int i = 0 ; i < page->header->num_of_keys; i++){
            if(page->keys[i] > delete_key){
                Page* next_page = page_read_at(tree->pm->fd, page->children[i]);
                bool deleted = btree_delete(tree, delete_key, next_page, page);
                return deleted;
            }
        }
    }

    Page* last_page = page_read_at(tree->pm->fd, page->children[page->header->num_of_keys]);
    return btree_delete(tree, delete_key, last_page, page);
}
