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
- Planned (soon)
  - Delete(key): delete a key from the btree

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
  - create_pagemanager(filename, trunc): create/open database and root page
  - get_pagemanager(filename): open an existing database
  - write_page(pm, page, offset): persist a page at a known offset
- Page
  - get_root_page(pm): read root page
  - get_page(fd): read a page at the current file offset
  - create_new_page(pm, keys, values, page_type): append a new page and return its offset
  - insert_key_in_internal_page, insert_key_in_leaf_page: helpers for in-page inserts
  - delete_key_from_page: helper for in-page deletion
  - print_page: debug dump
- Btree
  - init_btree(pm)
  - insert(btree, value, page, parent_page)
  - split_node(btree, page, parent_page)

Current behavior and limitations
- Keys in leaf pages are auto-incremented during insert and do not yet use the caller-provided key.
- Node splitting is implemented for both root and non-root pages. Internal nodes store child offsets (file positions) and separator keys.
- delete_key_from_page exists at the page level but there is no tree-level delete/rebalance logic.
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