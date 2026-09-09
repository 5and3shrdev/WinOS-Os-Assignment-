is# Windows OS Concept Model — Basic (All 4 Modules)

A simpler, broader version covering all four core Windows subsystems at a
basic level, rather than going deep on just two. Good if the rubric wants
coverage across more concepts even if each one is lighter.

## Modules

| Module | File | What it shows |
|---|---|---|
| Process/Thread Scheduling | `scheduler.c/h` | Priority-based scheduling: highest-priority ready process always runs next |
| Memory Management | `memory.c/h` | Demand paging with FIFO replacement, page faults vs. hits |
| File System | `filesystem.c/h` | NTFS-style folder tree, built from flat metadata records (like the MFT) |
| Synchronization | `sync.c/h` | Mutex (one owner) and Semaphore (N slots), with wait queues |

## Build & run

```
make
./os_model
```

Every module has a **Demo** option (no typing needed beyond menu numbers)
and a **Custom/Interactive** option for showing the teacher it isn't
hardcoded.

