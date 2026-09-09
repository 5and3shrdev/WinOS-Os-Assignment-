# Windows OS Concept Model — Basic (All 4 Modules)

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

## How this differs from the deep 2-module version

This version trades depth for breadth:

- **Scheduler**: non-preemptive, fixed priority, no dynamic boosting or
  starvation avoidance (the deep version has both).
- **Memory**: FIFO only, no LRU/Clock comparison (the deep version has all three).
- **File system & synchronization**: not modeled in the deep version at all —
  these are new here.

If your assignment only asked for depth on 1-2 modules, use the other
version instead; use this one if you need to show you understand all four
areas, even briefly.

## Written report

Same idea as before — the written report should explain the real Windows
OS broadly (architecture, scheduling, memory, file system, synchronization,
security), independent of which parts got modeled in code. This program
now gives you a small demonstration for all four sections instead of just two.
