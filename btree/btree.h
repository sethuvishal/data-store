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