#pragma once
#include <stdbool.h>

#include "page.h"

#define DB_HEADER_SIZE 100


typedef struct Page Page;


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
void write_page(PageManager* pm, Page* page, long offset);