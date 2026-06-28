Btree: a tiny disk‑backed B‑Tree in C

A compact, educational B‑Tree storage engine that persists fixed‑size pages to a single file. It shows how to:
- manage a paged on‑disk file
- insert with node splits (root and non‑root)
- look up, update, and delete keys with simple borrow/merge rebalancing

Status: prototype/learning project

Highlights
- Fixed‑size paging with a small database header
- Root page starts as a leaf and upgrades to an internal page on first split
- Keys are kept sorted inside pages; internal pages store child page offsets
- On‑disk format is intentionally simple and easy to inspect with a hex viewer

Repository layout
- btree/main.c: example program that builds and exercises a tree
- btree/btree.h, btree/btree.c: tree façade, insert/split/find/update/delete
- btree/page.h, btree/page.c: in‑memory page, (de)serialization, in‑page ops
- btree/pagemanager.h, btree/pagemanager.c: database file header + page I/O

Build and run
From repo root:

  gcc -std=c11 -Wall -Wextra -O2 btree/*.c -o btree-demo -lm
  ./btree-demo

The demo creates my.odb, inserts 1..5061 into the tree, updates key 34 to 100, deletes key 137, then prints a debug dump of the root and some children.

Quick usage (from C)
  #include "btree/btree.h"
  #include "btree/page.h"

  PageManager* pm = pagemanager_create("my.odb", true); // truncates/creates
  Btree* t = btree_create(pm);

  for (int i = 1; i <= 1000; i++) {
      btree_insert(t, i, NULL, NULL); // keys auto‑assigned in leaves
  }

  int v = btree_find(t, 42, NULL);           // returns stored value or 0
  (void)v;
  btree_update(t, 42, 999, NULL);            // updates value at key 42
  btree_delete(t, 42, NULL, NULL);           // deletes key 42

  Page* root = page_read_root(pm);           // read root page for debugging
  page_debug_print(root);

On‑disk format (simplified)
- Database header (DB_HEADER_SIZE = 100 bytes)
  struct PageManagerHeader {
    int page_count;  // not maintained yet
    bool empty;      // true if only root exists
    int page_size;   // must be 2048 in this build
  };

- Pages (PAGE_SIZE = 2048 bytes each)
  Reserved at start: PAGE_HEADER_SIZE = 50 bytes
  struct PageHeader {
    unsigned int page_type;   // bit flags: ROOT_PAGE | LEAF_PAGE | NON_LEAF_PAGE
    int          num_of_keys; // populated key count
    off_t        pos;         // absolute file offset of this page
  };

  Page body layout (fixed arrays, zero‑filled when unused):
  - keys[MAX_KEYS_PER_PAGE + 1]
  - either
    - data[MAX_KEYS_PER_PAGE + 2]      // for leaf pages (row values)
    - children[MAX_KEYS_PER_PAGE + 2]  // for internal pages (child offsets)

  Constants (page.h):
  - PAGE_SIZE = 2048
  - MAX_KEYS_PER_PAGE = 100            // so internal nodes can have up to 101 children
  - PAGE_HEADER_SIZE = 50              // reserved space at start of each page image

Programming model
- PageManager handles file creation/open and serializes full page images.
- Page encodes/decodes PageHeader + arrays to/from PAGE_SIZE buffers.
- Btree traverses pages and performs:
  - insert: append into leaf; split when full and push separator up
  - find: walk down using separator keys
  - update: find key in leaf and rewrite page
  - delete: remove key; borrow from siblings or merge when underflowing

Important behaviors and limitations
- Leaf keys are auto‑incremented during insert; the API currently accepts only a value for insert. You operate on explicit keys when calling find/update/delete.
- Crash‑safety: none (no WAL, no atomic page updates, no fsync protocol)
- Concurrency: none (single‑writer, single‑process only)
- Portability: no endianness conversion or packed struct guarantees beyond this build; page images are tied to the compiler/ABI used
- Space management: no free‑list; pages are only appended, not reclaimed
- Testing: no formal tests yet; main.c acts as a smoke test

How to experiment
- Change MAX_KEYS_PER_PAGE or PAGE_SIZE in btree/page.h to see different split/branching behavior
- Edit the insert loop in btree/main.c to change data distributions
- Inspect my.odb with hexdump or a hex editor to see page boundaries (2048‑byte steps)

Example: peek at headers (Linux/macOS)
  hexdump -C -n 100 my.odb            # DB header
  hexdump -C -s 100 -n 2048 my.odb    # root page image

Roadmap ideas
- Free‑list and page reuse; maintain page_count accurately
- Optional checksums; basic endianness handling; minimal crash‑recovery