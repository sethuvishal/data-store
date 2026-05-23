#include <stdlib.h>
#include <stdio.h>  
#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_KEYS 100

typedef uint32_t PageID;   // or uint64_t if you want huge files


typedef struct BTreeNode {
    int keys[MAX_KEYS];
    PageID children[MAX_KEYS + 1]; // PageIDs of child nodes
    int num_keys;
    int is_leaf;
} BTreeNode;


typedef struct BTree {
    PageID root_page_id;
    FILE* file; // File handle for the B-tree storage
} BTree;

void btree_initialize(FILE* file, BTree* btree_root){
    

}
