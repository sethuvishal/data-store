#pragma once
#include <stdbool.h>

#include "page.h"

#define PAGE_MANAGER_HEADER_SIZE 100


typedef struct PageManagerHeader {
    int page_count;
    bool empty;
    int page_size;
} PageManagerHeader;

typedef struct PageManager {
    PageManagerHeader* header;
    int fd;
} PageManager;


PageManager* create_pagemanager(const char* filename, bool trunc);
PageManager* get_pagemanager(const char* filename);