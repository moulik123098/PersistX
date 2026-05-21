# PersistX

A hands-on educational storage engine built in modern C++17.

PersistX demonstrates how real-world databases (PostgreSQL, SQLite, RocksDB) manage persistent storage from scratch — covering page layout, buffer management, write-ahead logging, crash recovery, and query indexing.

---

## Architecture

```
Client (TUI Menu)
        |
        v
  QueryEngine          <- sorted in-memory index (std::map)
        |
        v
  PageManager          <- orchestrates all storage operations
      /       \
     v         v
BufferPool   WALManager
     |             |
     v             v
DiskManager   wal.log
     |
     v
data/page_N.txt
```

Each layer has a single responsibility and a well-defined interface:

| Layer           | Responsibility                                         |
|-----------------|--------------------------------------------------------|
| **Record**      | Basic key-value unit of storage                        |
| **Page**        | Slotted page layout (4 KB, up to 64 records per page)  |
| **DiskManager** | Reads and writes individual page files from disk       |
| **BufferPool**  | In-memory page cache with LRU eviction (min-heap)      |
| **WALManager**  | Write-ahead logging for crash durability               |
| **PageManager** | Coordinates inserts, deletes, WAL recovery on startup  |
| **QueryEngine** | Sorted index supporting get, prefix filter, range scan |

---

## Features

- **Page-based storage** — all records stored in fixed 4 KB pages (64 bytes per record, 64 records per page)
- **Buffer pool with LRU eviction** — min-heap-based LRU cache with configurable capacity (default: 10 pages); dirty pages are flushed before eviction
- **Write-Ahead Logging (WAL)** — every INSERT and REMOVE is logged before it touches data; supports checkpoint and pending-entry recovery
- **Crash recovery** — on startup, PageManager detects un-checkpointed WAL entries and replays them to restore a consistent state
- **Crash simulation** — option to abruptly exit without flushing, letting you test WAL recovery on the next run
- **In-memory sorted index** — `QueryEngine` maintains a `std::map` from keys to `(pageId, slotIndex)` for O(log n) lookups
- **Query operations** — get by key, list all records, prefix filter, lexicographic range query
- **Debug TUI** — ANSI-coloured terminal interface with a live event log panel showing the last 6 storage-layer events

---

## Project Structure

```
PersistX/
├── include/
│   ├── Record.h          # Key-value record struct
│   ├── Page.h            # Slotted page layout and operations
│   ├── DiskManager.h     # Page file I/O
│   ├── BufferPool.h      # LRU page cache
│   ├── WALManager.h      # Write-ahead log
│   ├── PageManager.h     # Storage coordinator + crash recovery
│   └── QueryEngine.h     # Sorted index and query operations
├── src/
│   ├── main.cpp          # Debug TUI entry point
│   ├── Page.cpp
│   ├── DiskManager.cpp
│   ├── BufferPool.cpp
│   ├── WALManager.cpp
│   ├── PageManager.cpp
│   └── QueryEngine.cpp
├── CMakeLists.txt
└── .gitignore
```

---

## Building

**Prerequisites:** CMake >= 3.16, a C++17-compatible compiler (GCC, Clang, or MSVC).

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build
```

On Windows with MinGW:
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

The compiled binary lands at `build/db_engine` (or `build/db_engine.exe` on Windows).

---

## Running

```bash
./build/db_engine
```

The TUI menu launches automatically. On startup, the engine checks for un-checkpointed WAL entries and replays them before accepting input.

---

## Menu Options

```
RECORDS
  1. Insert record
  2. Remove record
  3. Get by key
  4. List all records

QUERIES
  5. Filter by prefix
  6. Range query

INDEX
  7. Show index
  8. Rebuild index

STORAGE
  9.  Display all pages
  10. Buffer pool stats
  11. Flush to disk

DANGER ZONE
  12. Crash  (simulate abrupt exit -- WAL NOT flushed)
  13. Exit   (safe flush + quit)
```

---

## How WAL Recovery Works

```
Normal run:
  INSERT key --> WAL logs INSERT --> Page updated --> Buffer marked dirty
  EXIT (option 13) --> flushAll() --> WAL CHECKPOINT written

Crash run:
  INSERT key --> WAL logs INSERT --> Page updated
  CRASH (option 12) --> process exits; no checkpoint, no flush

Next startup:
  DiskManager restores pages from disk
  WALManager detects pending entries (after last CHECKPOINT)
  PageManager replays all pending INSERTs and REMOVEs --> data restored
  New CHECKPOINT written --> engine ready
```

---

## Design Principles

1. **Page-based I/O** — the unit of all reads and writes is a fixed 4 KB page
2. **Buffer-first writes** — data is mutated in the buffer pool; disk writes happen on eviction or flush
3. **WAL before data** — the log is always written and flushed before any page hits disk
4. **Crash-first design** — the system is always recoverable to a consistent state after an abrupt exit
5. **Index consistency** — `QueryEngine` updates its in-memory index on every insert and remove, and can be fully rebuilt from the buffer pool at any time

---

## Mentees

| Name            | Repository                                      |
|-----------------|-------------------------------------------------|
| Ekansh Goel     | https://github.com/ekansh1905/PersistX.git      |
| Moulik Bansal   | https://github.com/moulik123098/PersistX.git    |
| Krivit Sisodiya | https://github.com/krix-s/PersistX.git          |

---

## License

This project is for educational purposes.
