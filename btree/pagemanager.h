#pragma once
#include <stdbool.h>

#include "page.h"

#define DB_HEADER_SIZE 100

typedef struct Page Page;

// On-disk database file header written at offset 0..DB_HEADER_SIZE-1.
typedef struct PageManagerHeader {
    int page_count;   // optional bookkeeping (not yet maintained)
    bool empty;       // true if only the root page exists
    int page_size;    // must match PAGE_SIZE
} PageManagerHeader;

// Thin wrapper around the database file descriptor and header state.
typedef struct PageManager {
    PageManagerHeader* header;
    int fd;
} PageManager;

// Create a new database file (trunc == true) or fail if it exists (trunc == false).
// Also writes an empty root page after the DB header.
PageManager* pagemanager_create(const char* filename, bool trunc);

// Open an existing database file and load its header.
PageManager* pagemanager_open(const char* filename);

// Serialize a Page back to disk at the given absolute byte offset.
void pagemanager_write_page(PageManager* pm, Page* page, long offset);
