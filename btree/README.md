Btree - a disk-backed B-Tree experiment in C

Minimal B-Tree–style storage engine that persists nodes (pages) to disk. It demonstrates fixed-size paging, a simple on-disk layout, and node splitting during inserts.

Status: early prototype
- Implemented
  - Fixed-size page manager with a simple file header
  - Page structure with headers, keys, and either children pointers or row data
  - Root page creation on a fresh database file
  - Insert that appends values into leaf pages and splits nodes when full
  - On-disk persistence and basic page I/O
  - Debug printing of pages
  - find(key): look up by key
  - update(key, value): update a stored value by key
  - delete(key): delete a key from the btree

Repository layout
- btree/
  - main.c: demo program that creates a database and inserts sample values
  - btree.h / btree.c: B-Tree façade and core insert/split logic
  - page.h / page.c: in-memory page representation and (de)serialization helpers
  - pagemanager.h / pagemanager.c: database file management and page read/write

Build and run
From the repository root:

  gcc -std=c11 -Wall -Wextra -O2 btree/*.c -o btree-demo
  ./btree-demo

The demo creates my.odb, inserts 150 integer values, and prints the resulting pages to stdout.

On-disk layout (simplified)
- File header (DB_HEADER_SIZE = 100 bytes)
  - PageManagerHeader { page_count, empty, page_size }
- Pages (PAGE_SIZE = 2048 bytes each)
  - PageHeader (PAGE_HEADER_SIZE = 50 bytes reserved in the page image)
    - page_type: bit flags (ROOT_PAGE, LEAF_PAGE, NON_LEAF_PAGE)
    - num_of_keys: number of populated keys
    - pos: byte offset of this page in the file
  - keys: MAX_KEYS_PER_PAGE + 1 ints
  - payload:
    - if LEAF_PAGE: data[MAX_KEYS_PER_PAGE + 2] (row values)
    - else: children[MAX_KEYS_PER_PAGE + 2] (file offsets to child pages)

Core data structures (high level)
- PageManager
  - pagemanager_create(filename, trunc): create/open database and root page
  - pagemanager_open(filename): open an existing database
  - pagemanager_write_page(pm, page, offset): persist a page at a known offset
- Page
  - page_read_root(pm): read root page
  - page_read_at(fd): read a page at the given file offset
  - page_create_and_append(pm, keys, values, page_type): append a new page and return its offset
  - page_insert_internal, page_insert_leaf: helpers for in-page inserts
  - page_delete_key: helper for in-page deletion
  - page_debug_print: debug dump
- Btree
  - btree_create(pm)
  - btree_insert(tree, value, page, parent)
  - btree_split_node(tree, page, parent)
  - btree_find(tree, key, page)
  - btree_update(tree, key, value, page)
  - btree_delete(tree, key, page, parent)

Current behavior and limitations
- Keys in leaf pages are auto-incremented during insert and do not yet use the caller-provided key.
- Node splitting is implemented for both root and non-root pages. Internal nodes store child offsets (file positions) and separator keys.
- Concurrency, crash safety (write-ahead logging), checksums, schema/catalog management, and free page reuse are out of scope for this prototype.
- Portability notes: pages are written via raw memcpy of in-memory structs; there is no endianness handling or struct packing guarantees beyond this single build environment.

Roadmap
- Tree-level delete with merge/redistribute and underflow handling
- Free list and page reuse; page_count tracking and sanity checks
- Unit and fuzz tests; property-based checks for split/merge invariants

How to experiment
- Tweak MAX_KEYS_PER_PAGE and PAGE_SIZE in btree/page.h and observe split behavior
- Change the insert loop in btree/main.c to insert different distributions and sizes
- Inspect odb file with a hex viewer to see the page images
