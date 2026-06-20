#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "page.h"
#include "pagemanager.h"


PageManager* create_pagemanager(const char* filename, bool trunc){
    int flags = O_CREAT | O_RDWR;

    if (trunc) {
        flags |= O_TRUNC;
    } else {
        flags |= O_EXCL;
    }
    int fd = open(filename, flags, 0644);

    if (fd == -1) {
        perror("Error");
        return NULL;
    }

    PageManager* pm = (PageManager*) malloc(sizeof(PageManager));

    if (pm == NULL) {
        perror("malloc");
        close(fd);
        return NULL;
    }
    PageManagerHeader* header = (PageManagerHeader*) malloc(sizeof(PageManagerHeader));

    if (header == NULL) {
        perror("malloc");
        free(pm);
        close(fd);
        return NULL;
    }

    // setting default values to page manager header
    header->empty = true;
    header->page_count = 0;
    header->page_size = PAGE_SIZE;

    pm->fd = fd;
    pm->header = header;

    PageHeader* root_page_header = malloc(sizeof(PageHeader));
    if(root_page_header == NULL){
        perror("malloc");
        close(fd);
        free(header);
        free(pm);
        return NULL;
    }
    root_page_header->page_type = ROOT_PAGE | LEAF_PAGE;
    root_page_header->num_of_keys = 0;

    ssize_t written;

    /* write page manager header */
    char buffer[100] = {0};

    memcpy(buffer, header, sizeof(PageManagerHeader));

    written = write(fd, buffer, sizeof(buffer));

    if (written != sizeof(buffer)) {
        perror("write header");
        close(fd);
        free(root_page_header);
        free(header);
        free(pm);
        return NULL;
    }

    uint8_t page_buf[PAGE_SIZE];
    memset(page_buf, 0, PAGE_SIZE);

    /* copy page header into beginning of page */
    root_page_header->pos = lseek(fd, 0, SEEK_CUR);
    memcpy(page_buf, root_page_header, sizeof(PageHeader));
    written = write(fd, page_buf, PAGE_SIZE);

    if (written != PAGE_SIZE) {
        perror("write root page");
        close(fd);
        free(root_page_header);
        free(header);
        free(pm);
        return NULL;
    }

    return pm;
}

void write_page(PageManager* pm, Page* page, long offset){
    int fd = pm->fd;

    if(lseek(fd, offset, SEEK_SET) == -1){
        perror("lseek");
        return;
    }

    uint8_t page_buf[PAGE_SIZE];
    memset(page_buf, 0, PAGE_SIZE);
    memcpy(page_buf, page->header, sizeof(PageHeader));
    memcpy(page_buf + PAGE_HEADER_SIZE, page->keys, sizeof(page->keys));
    memcpy(page_buf + PAGE_HEADER_SIZE + sizeof(page->keys), page->children, sizeof(page->children));

    int written = write(fd, page_buf, PAGE_SIZE);
    if(written == -1){
        perror("write page failed");
        return;
    }
    return;
}

PageManager* get_pagemanager(const char* filename){
    int fd = open(filename, O_RDWR);

    if(fd == -1){
        fprintf(stderr, "Error: Failed to open file (%s).\n", filename);
        return NULL;
    }

    uint8_t buffer[DB_HEADER_SIZE];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return NULL;
    }

    if (bytes_read < sizeof(PageManager)) {
        fprintf(stderr, "Error: File does not contain enough data.\n");
        close(fd);
        return NULL;
    }

    PageManager* pm = (PageManager*) malloc(sizeof(PageManager));
    if (pm == NULL){
        perror("malloc");
        close(fd);
        return NULL;
    }

    PageManagerHeader* header = (PageManagerHeader*) malloc(sizeof(PageManagerHeader));
    if (header == NULL){
        perror("malloc");
        free(pm);
        close(fd);
        return NULL;
    }
    // first 100 bytes of data is reserver for Header.
    // Reading first 100 bytes of data and update header
    memcpy(header, buffer, sizeof(PageManagerHeader));
    pm->fd = fd;
    pm->header = header;

    return pm;
}