# data-store

A implementation project to build core data stores used in real‑world databases. The goal is to design, implement, benchmark, and document fundamental storage engines and file formats from scratch.

What this project will cover:
- B‑Tree / B+Tree
  - [x] Fixed-size pages and on-disk page layout
  - [] B+ tree nodes (internal + leaf) using page IDs instead of pointers
  - [x] Core operations: search, insert, split, and delete
  - [x] Persistence: read/write pages and recover the tree after restart\
- LSM‑Tree
  - Write path: WAL + memtable
  - Compaction strategies (tiered/leveled)
  - Read path optimizations (Bloom filters, block cache)
- SSTables (Sorted String Tables)
  - Immutable, append‑only table format
  - Data blocks, index blocks, filters, metadata
  - Compression and checksums

Objectives:
- Re‑implement these systems with clear, well‑documented abstractions
- Compare design trade‑offs (read/write/space amplification, latency, throughput)
- Provide minimal reference implementations with tests and benchmarks
- Explain internals with comments and diagrams where useful

Planned structure:
- btree/: B‑Tree/B+Tree storage engine
- lsm/: LSM‑Tree storage engine (memtable, WAL, compaction)
- sst/: SSTable reader/writer library (blocks, index, filters)

Roadmap (high level):
- [ ] Define common KeyValueStore interfaces and on‑disk format conventions
- [ ] Implement SSTable writer/reader and basic tooling
- [ ] Add Bloom filter and block cache
- [ ] Build LSM write path (WAL + memtable) and background compaction
- [ ] Implement B+Tree with pager and buffer pool
- [ ] Add benchmarks and trace‑based tests to compare engines
- [ ] Document performance characteristics and tuning knobs

Status: early work‑in‑progress. See subdirectories for current progress.
