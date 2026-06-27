#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "pagemanager.h"

typedef struct Btree {
    PageManager* pm;   // backing page manager (owns fd and file header)
    long count;        // optional future-use; not enforced in this prototype
} Btree;

// Construct a B-Tree around an existing PageManager.
Btree* btree_create(PageManager* pm);

// Insert a value. If page/parent are NULL, traversal starts at the root.
void btree_insert(Btree* tree, int value, Page* page, Page* parent);

// Split a full node and push separator up. Internal helper, but kept public
// for now to ease experimentation.
void btree_split_node(Btree* tree, Page* page, Page* parent);

// Look up a key; returns 0 if not found (prototype behavior).
int btree_find(Btree* tree, int key, Page* page);

// Update an existing key with a new value; returns true if updated.
bool btree_update(Btree* tree, int key, int new_value, Page* page);

// Delete a key and rebalance (borrow/merge) if needed; returns true on success.
bool btree_delete(Btree* tree, int key, Page* page, Page* parent);

// Rebalance helpers used during delete.
bool btree_borrow_from_left_sibling(Btree* tree, Page* parent, Page* current);
bool btree_borrow_from_right_sibling(Btree* tree, Page* parent, Page* current);
