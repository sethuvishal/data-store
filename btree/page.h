#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "pagemanager.h"

// Tunables for this prototype.
#define PAGE_SIZE 2048
#define MAX_KEYS_PER_PAGE 100
#define PAGE_HEADER_SIZE 50   // bytes reserved at page start for PageHeader image

typedef struct PageManager PageManager;

// Page types are bit-flags so a root can also be a leaf initially.
typedef enum PAGE_TYPE {
    ROOT_PAGE    = 1 << 0,
    LEAF_PAGE    = 1 << 1,
    NON_LEAF_PAGE= 1 << 2
} PAGE_TYPE;

// In-memory representation of the fixed-size on-disk page header.
typedef struct PageHeader {
    unsigned int page_type;  // bitwise OR of PAGE_TYPE flags
    int num_of_keys;         // number of valid keys in this page
    off_t pos;               // absolute file offset where this page is stored
} PageHeader;

// In-memory representation of a page image.
typedef struct Page{
    PageHeader* header;
    int keys[MAX_KEYS_PER_PAGE + 1];
    union {
        int children[MAX_KEYS_PER_PAGE + 2]; // for internal pages: child page offsets
        int data[MAX_KEYS_PER_PAGE + 2];     // for leaf pages: stored values
    };
} Page;

// Read a page located at absolute file offset `offset`.
Page* page_read_at(int fd, ssize_t offset);

// Convenience: read the current root page (located right after DB header).
Page* page_read_root(PageManager* pm);

// Debug pretty-printer for a Page.
void page_debug_print(Page* page);

// In-page operations (do not write to disk themselves).
void page_insert_internal(Page* page, int key, int value);
void page_insert_leaf(Page* page, int key, int value);
void page_delete_key(Page* page, int key);

// Allocate and append a new page at EOF with provided contents. Returns its file offset.
off_t page_create_and_append(PageManager* pm, int* keys, int* values, unsigned int page_type);
