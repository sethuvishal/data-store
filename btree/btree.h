#include <stdlib.h>
#include <stdio.h>  
#include <stdint.h>
#include "pagemanager.h"


typedef struct Btree {
    PageManager* pm;
    long count;
} Btree;

Btree* init_btree(PageManager* pm);
void insert(Btree* btree, int value, Page* page, Page* parent_page);
void split_node(Btree* btree, Page* page, Page* parent_page);

int find(Btree* btree, int search_key, Page* page);
bool update(Btree* btree, int update_key, int new_value, Page* page);
bool delete(Btree* btree, int delete_key, Page* page, Page* parent_page);

bool borrow_from_left_page(Btree* btree, Page* parent_page, Page* current_page);
bool borrow_from_right_page(Btree* btree, Page* parent_page, Page* current_page);