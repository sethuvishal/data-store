#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "pagemanager.h"

#define PAGE_SIZE 2048
#define MAX_KEYS_PER_PAGE 100
#define PAGE_HEADER_SIZE 50

typedef struct PageManager PageManager;

typedef enum PAGE_TYPE {
    ROOT_PAGE = 1 << 0,
    LEAF_PAGE = 1 << 1,
    NON_LEAF_PAGE = 1 << 2
} PAGE_TYPE;

typedef struct PageHeader {
    unsigned int page_type;
    int num_of_keys;
} PageHeader;

typedef struct Page{
    PageHeader* header;
    int keys[MAX_KEYS_PER_PAGE + 1];
    union {
        int children[MAX_KEYS_PER_PAGE + 2];
        int data[MAX_KEYS_PER_PAGE + 1];
    };
} Page;

Page* get_page(int fd);
Page* get_root_page(PageManager* pm);
void print_page(Page* page);
void insert_key_in_internal_page(Page* page, int key, int value);
void insert_key_in_leaf_page(Page* page, int key, int value);
void delete_key_from_page(Page* page, int key);