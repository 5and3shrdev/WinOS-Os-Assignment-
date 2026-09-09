/*
 * ================================================================
 *  Windows OS Concept Model -- All 4 Core Subsystems (SINGLE FILE)
 * ================================================================
 *
 * A basic C program that simulates four core Windows OS mechanisms:
 *   1. Process/Thread Scheduling  -- priority-based, non-preemptive
 *   2. Memory Management          -- demand paging with FIFO replacement
 *   3. File System                -- NTFS-style folder tree (MFT-style records)
 *   4. Synchronization            -- Mutex and Semaphore with wait queues
 *
 * This is a concept demonstration, not a real OS -- no bootloader, no
 * hardware access. It models the *algorithms* real Windows uses, on top
 * of a normal C program running under whatever OS compiles it.
 *
 * Build:  gcc -o os_model windows_os_model.c
 * Run:    ./os_model        (or os_model.exe on Windows)
 */

#include <stdio.h>
#include <string.h>

/* ================================================================
 *  MODULE 1: PROCESS / THREAD SCHEDULING
 * ================================================================
 * Basic model of Windows-style priority scheduling: the highest-priority
 * ready process always runs, chosen non-preemptively (runs to completion
 * once picked); ties are broken by earliest arrival.
 */

#define MAX_PROCS 20

typedef struct {
    char name[32];
    int priority;   /* Windows-style base priority: 24,13,10,8,6,4 (higher = runs first) */
    int arrival;
    int burst;
    int done;
    int start;
    int completion;
} Proc;

static Proc procs[MAX_PROCS];
static int n = 0;

static const char *class_name(int p) {
    switch (p) {
        case 24: return "Realtime";
        case 13: return "High";
        case 10: return "Above Normal";
        case 8:  return "Normal";
        case 6:  return "Below Normal";
        case 4:  return "Idle";
        default: return "Custom";
    }
}

static void print_table(void) {
    printf("\n  %-10s %-14s %-6s %-6s %-6s %-10s %-8s %-10s\n",
           "Process", "Class", "Arr", "Burst", "Start", "Completion", "Waiting", "Turnaround");
    int total_wait = 0, total_ta = 0;
    for (int i = 0; i < n; i++) {
        int wait = procs[i].start - procs[i].arrival;
        int ta = procs[i].completion - procs[i].arrival;
        total_wait += wait; total_ta += ta;
        printf("  %-10s %-14s %-6d %-6d %-6d %-10d %-8d %-10d\n",
               procs[i].name, class_name(procs[i].priority), procs[i].arrival, procs[i].burst,
               procs[i].start, procs[i].completion, wait, ta);
    }
    printf("\n  Average waiting time   : %.2f\n", (double)total_wait / n);
    printf("  Average turnaround time: %.2f\n", (double)total_ta / n);
}

static void run_priority_nonpreemptive(void) {
    int time = 0, done_count = 0;
    for (int i = 0; i < n; i++) procs[i].done = 0;

    printf("\n  Gantt chart: ");
    while (done_count < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (procs[i].done || procs[i].arrival > time) continue;
            if (best == -1 || procs[i].priority > procs[best].priority ||
                (procs[i].priority == procs[best].priority && procs[i].arrival < procs[best].arrival))
                best = i;
        }
        if (best == -1) { time++; continue; }   /* nothing has arrived yet */
        procs[best].start = time;
        time += procs[best].burst;
        procs[best].completion = time;
        procs[best].done = 1;
        done_count++;
        printf("[%s %d-%d] ", procs[best].name, procs[best].start, procs[best].completion);
    }
    printf("\n");
    print_table();
}

void scheduler_run_demo(void) {
    n = 0;
    strcpy(procs[n].name, "Explorer"); procs[n].priority = 10; procs[n].arrival = 0; procs[n].burst = 5; n++;
    strcpy(procs[n].name, "Backup");   procs[n].priority = 6;  procs[n].arrival = 0; procs[n].burst = 8; n++;
    strcpy(procs[n].name, "AV_Scan");  procs[n].priority = 8;  procs[n].arrival = 2; procs[n].burst = 6; n++;
    strcpy(procs[n].name, "SysMon");   procs[n].priority = 13; procs[n].arrival = 3; procs[n].burst = 3; n++;

    printf("\n  Demo: Windows-style priority scheduling.\n");
    printf("  The highest-priority ready process always runs next.\n");
    run_priority_nonpreemptive();
}

void scheduler_run_custom(void) {
    printf("  How many processes? ");
    if (scanf("%d", &n) != 1) return;
    if (n > MAX_PROCS) n = MAX_PROCS;

    for (int i = 0; i < n; i++) {
        printf("\n  Process %d name: ", i + 1);
        scanf("%31s", procs[i].name);
        printf("  Priority class (24=Realtime 13=High 10=AboveNormal 8=Normal 6=BelowNormal 4=Idle): ");
        scanf("%d", &procs[i].priority);
        printf("  Arrival time: ");
        scanf("%d", &procs[i].arrival);
        printf("  Burst time: ");
        scanf("%d", &procs[i].burst);
    }
    run_priority_nonpreemptive();
}

/* ================================================================
 *  MODULE 2: MEMORY MANAGEMENT (PAGING)
 * ================================================================
 * Basic model of Windows demand paging: a virtual page is loaded into
 * physical memory the first time it's touched (a page fault); once memory
 * is full, the oldest-loaded page is evicted (FIFO).
 */

#define MAX_FRAMES 16
#define MAX_REFS 100

static int run_fifo(const int *refs, int nref, int nframes, int verbose) {
    int frames[MAX_FRAMES];
    for (int i = 0; i < nframes; i++) frames[i] = -1;
    int next_victim = 0;
    int faults = 0;

    for (int t = 0; t < nref; t++) {
        int p = refs[t];
        int hit = -1;
        for (int f = 0; f < nframes; f++) if (frames[f] == p) { hit = f; break; }

        if (hit == -1) {
            faults++;
            frames[next_victim] = p;
            if (verbose) printf("  ref %2d (page %2d) [FAULT] -> loaded into frame %d\n", t, p, next_victim);
            next_victim = (next_victim + 1) % nframes;
        } else if (verbose) {
            printf("  ref %2d (page %2d) [HIT]\n", t, p);
        }

        if (verbose) {
            printf("     frames: ");
            for (int f = 0; f < nframes; f++) { if (frames[f] == -1) printf(" . "); else printf("%2d ", frames[f]); }
            printf("\n");
        }
    }
    return faults;
}

void memory_run_demo(void) {
    int refs[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    int nref = (int)(sizeof(refs) / sizeof(refs[0]));
    int nframes = 3;

    printf("\n  Demo: virtual pages 1-5, physical memory = %d frames, FIFO replacement.\n", nframes);
    printf("  Pages load on first access and the oldest-loaded page is evicted\n");
    printf("  once memory fills up -- the basic idea behind demand paging.\n\n");

    int faults = run_fifo(refs, nref, nframes, 1);
    printf("\n  Total page faults: %d / %d references (hit ratio %.2f)\n",
           faults, nref, 1.0 - (double)faults / nref);
}

void memory_run_custom(void) {
    int nframes, nref;
    printf("  Number of physical frames: ");
    if (scanf("%d", &nframes) != 1) return;
    if (nframes < 1) nframes = 1;
    if (nframes > MAX_FRAMES) nframes = MAX_FRAMES;

    printf("  How many page references? ");
    if (scanf("%d", &nref) != 1) return;
    if (nref < 1) nref = 1;
    if (nref > MAX_REFS) nref = MAX_REFS;

    int refs[MAX_REFS];
    printf("  Enter %d page numbers, separated by spaces: ", nref);
    for (int i = 0; i < nref; i++) scanf("%d", &refs[i]);

    int faults = run_fifo(refs, nref, nframes, 1);
    printf("\n  Total page faults: %d / %d references (hit ratio %.2f)\n",
           faults, nref, 1.0 - (double)faults / nref);
}

/* ================================================================
 *  MODULE 3: FILE SYSTEM (NTFS-STYLE TREE)
 * ================================================================
 * Basic model of an NTFS-style file system: a hierarchy of files and
 * folders is really just a flat table of metadata records, each pointing
 * to its parent -- the same idea as NTFS's Master File Table (MFT).
 */

#define MAX_NODES 200
#define NAME_LEN 32

typedef struct {
    char name[NAME_LEN];
    int is_folder;
    int size_kb;
    int parent;   /* -1 for root */
    int used;
} Node;

static Node nodes[MAX_NODES];
static int node_count = 0;
static int current_dir = 0;

static int create_node(const char *name, int is_folder, int size_kb, int parent) {
    if (node_count >= MAX_NODES) return -1;
    int id = node_count++;
    strncpy(nodes[id].name, name, NAME_LEN - 1);
    nodes[id].name[NAME_LEN - 1] = '\0';
    nodes[id].is_folder = is_folder;
    nodes[id].size_kb = size_kb;
    nodes[id].parent = parent;
    nodes[id].used = 1;
    return id;
}

static void reset_fs(void) {
    node_count = 0;
    current_dir = create_node("C:", 1, 0, -1);
}

static void print_path(int id) {
    if (id == -1) return;
    if (nodes[id].parent != -1) { print_path(nodes[id].parent); printf("\\"); }
    printf("%s", nodes[id].name);
}

static void list_dir(int dir) {
    printf("\n  Directory of "); print_path(dir); printf("\n\n");
    int any = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].used && nodes[i].parent == dir) {
            any = 1;
            if (nodes[i].is_folder) printf("    <DIR>       %s\n", nodes[i].name);
            else printf("    %6d KB  %s\n", nodes[i].size_kb, nodes[i].name);
        }
    }
    if (!any) printf("    (empty)\n");
}

static int find_child(int dir, const char *name) {
    for (int i = 0; i < node_count; i++)
        if (nodes[i].used && nodes[i].parent == dir && strcmp(nodes[i].name, name) == 0) return i;
    return -1;
}

static void delete_recursive(int id) {
    for (int i = 0; i < node_count; i++)
        if (nodes[i].used && nodes[i].parent == id) delete_recursive(i);
    nodes[id].used = 0;
}

static void print_mft(void) {
    printf("\n  MFT-style record table (one metadata record per file/folder):\n");
    printf("  %-4s %-16s %-8s %-8s %-8s\n", "ID", "Name", "Type", "SizeKB", "ParentID");
    for (int i = 0; i < node_count; i++) {
        if (!nodes[i].used) continue;
        printf("  %-4d %-16s %-8s %-8d %-8d\n", i, nodes[i].name,
               nodes[i].is_folder ? "Folder" : "File", nodes[i].is_folder ? 0 : nodes[i].size_kb, nodes[i].parent);
    }
}

static void interactive_fs_menu(void) {
    int choice;
    do {
        printf("\n  Current directory: "); print_path(current_dir); printf("\n");
        printf("  1.Create folder  2.Create file  3.Delete  4.List dir  5.Enter folder\n");
        printf("  6.Go up (..)     7.Show MFT table  0.Back\n  Choice: ");
        if (scanf("%d", &choice) != 1) break;

        char name[NAME_LEN];
        int id, size;
        switch (choice) {
            case 1:
                printf("  Folder name: "); scanf("%31s", name);
                if (find_child(current_dir, name) != -1) printf("  Already exists.\n");
                else create_node(name, 1, 0, current_dir);
                break;
            case 2:
                printf("  File name: "); scanf("%31s", name);
                printf("  Size (KB): "); scanf("%d", &size);
                if (find_child(current_dir, name) != -1) printf("  Already exists.\n");
                else create_node(name, 0, size, current_dir);
                break;
            case 3:
                printf("  Name to delete: "); scanf("%31s", name);
                id = find_child(current_dir, name);
                if (id == -1) printf("  Not found.\n");
                else { delete_recursive(id); printf("  Deleted.\n"); }
                break;
            case 4:
                list_dir(current_dir);
                break;
            case 5:
                printf("  Folder to enter: "); scanf("%31s", name);
                id = find_child(current_dir, name);
                if (id == -1 || !nodes[id].is_folder) printf("  No such folder.\n");
                else current_dir = id;
                break;
            case 6:
                if (nodes[current_dir].parent != -1) current_dir = nodes[current_dir].parent;
                else printf("  Already at root.\n");
                break;
            case 7:
                print_mft();
                break;
        }
    } while (choice != 0);
}

void filesystem_run_demo(void) {
    reset_fs();
    printf("\n  Demo: building a small NTFS-style folder tree...\n");

    int docs = create_node("Documents", 1, 0, current_dir);
    int pics = create_node("Pictures", 1, 0, current_dir);
    create_node("resume.docx", 0, 45, docs);
    create_node("notes.txt", 0, 3, docs);
    create_node("vacation.jpg", 0, 2200, pics);

    list_dir(current_dir);
    printf("\n  Entering Documents...\n");
    list_dir(docs);
    print_mft();

    printf("\n  Notice every file/folder is one flat record with a parent pointer --\n");
    printf("  that's the same idea NTFS uses in its Master File Table.\n");
}

void filesystem_run_interactive(void) {
    reset_fs();
    printf("\n  Build your own folder tree using the menu below.\n");
    interactive_fs_menu();
}

/* ================================================================
 *  MODULE 4: SYNCHRONIZATION (MUTEX / SEMAPHORE)
 * ================================================================
 * Basic model of two Windows synchronization objects (accessed through
 * handles in the real OS): a Mutex, which only one thread may own at a
 * time, and a Semaphore, which lets up to N threads through at once.
 */

#define MAX_WAIT 10

typedef struct {
    int locked;
    char owner[32];
    char wait_queue[MAX_WAIT][32];
    int wait_count;
} Mutex;

static void mutex_init(Mutex *m) { m->locked = 0; m->owner[0] = '\0'; m->wait_count = 0; }

static void mutex_acquire(Mutex *m, const char *thread) {
    if (!m->locked) {
        m->locked = 1;
        strcpy(m->owner, thread);
        printf("  %-10s acquires the mutex immediately.\n", thread);
    } else {
        strcpy(m->wait_queue[m->wait_count++], thread);
        printf("  %-10s finds the mutex locked by %s -> blocks (added to wait queue).\n", thread, m->owner);
    }
}

static void mutex_release(Mutex *m, const char *thread) {
    if (strcmp(m->owner, thread) != 0) {
        printf("  %-10s tried to release a mutex it doesn't own -- ignored.\n", thread);
        return;
    }
    printf("  %-10s releases the mutex.\n", thread);
    if (m->wait_count > 0) {
        strcpy(m->owner, m->wait_queue[0]);
        for (int i = 1; i < m->wait_count; i++) strcpy(m->wait_queue[i - 1], m->wait_queue[i]);
        m->wait_count--;
        printf("  %-10s is woken up and now owns the mutex.\n", m->owner);
    } else {
        m->locked = 0;
        m->owner[0] = '\0';
    }
}

typedef struct {
    int count;
    char wait_queue[MAX_WAIT][32];
    int wait_count;
} Semaphore;

static void sem_init(Semaphore *s, int initial) { s->count = initial; s->wait_count = 0; }

static void sem_acquire(Semaphore *s, const char *thread) {
    if (s->count > 0) {
        s->count--;
        printf("  %-10s acquires the semaphore (count now %d).\n", thread, s->count);
    } else {
        strcpy(s->wait_queue[s->wait_count++], thread);
        printf("  %-10s finds the semaphore at 0 -> blocks (added to wait queue).\n", thread);
    }
}

static void sem_release(Semaphore *s, const char *thread) {
    printf("  %-10s releases the semaphore.\n", thread);
    if (s->wait_count > 0) {
        char woken[32];
        strcpy(woken, s->wait_queue[0]);
        for (int i = 1; i < s->wait_count; i++) strcpy(s->wait_queue[i - 1], s->wait_queue[i]);
        s->wait_count--;
        printf("  %-10s is woken up and proceeds (count stays %d).\n", woken, s->count);
    } else {
        s->count++;
        printf("  (no one waiting; count is now %d)\n", s->count);
    }
}

void sync_run_demo(void) {
    printf("\n  Demo A: Mutex -- two threads competing for one shared resource.\n");
    printf("  (Only the current owner may release a Windows Mutex.)\n\n");
    Mutex m; mutex_init(&m);
    mutex_acquire(&m, "Thread-A");
    mutex_acquire(&m, "Thread-B");
    mutex_release(&m, "Thread-A");

    printf("\n  Demo B: Semaphore -- a resource pool that allows 2 threads at once.\n\n");
    Semaphore s; sem_init(&s, 2);
    sem_acquire(&s, "Worker-1");
    sem_acquire(&s, "Worker-2");
    sem_acquire(&s, "Worker-3");   /* pool is full -> blocks */
    sem_release(&s, "Worker-1");   /* wakes Worker-3 */
}

void sync_run_interactive(void) {
    printf("\n  1. Mutex demo (interactive)\n  2. Semaphore demo (interactive)\n  Choice: ");
    int choice;
    if (scanf("%d", &choice) != 1) return;

    int op;
    char name[32];
    printf("  Enter 0 to stop. Otherwise 1=acquire 2=release, then a thread name.\n");

    if (choice == 1) {
        Mutex m; mutex_init(&m);
        while (1) {
            printf("  Operation: ");
            if (scanf("%d", &op) != 1 || op == 0) break;
            printf("  Thread name: "); scanf("%31s", name);
            if (op == 1) mutex_acquire(&m, name); else mutex_release(&m, name);
        }
    } else {
        int initial;
        printf("  Initial semaphore count: "); scanf("%d", &initial);
        Semaphore s; sem_init(&s, initial);
        while (1) {
            printf("  Operation: ");
            if (scanf("%d", &op) != 1 || op == 0) break;
            printf("  Thread name: "); scanf("%31s", name);
            if (op == 1) sem_acquire(&s, name); else sem_release(&s, name);
        }
    }
}

/* ================================================================
 *  MAIN MENU
 * ================================================================ */

static void banner(void) {
    printf("\n================================================================\n");
    printf("        Windows OS Concept Model -- All 4 Core Subsystems\n");
    printf("================================================================\n");
}

int main(void) {
    int choice = -1;

    do {
        banner();
        printf("  1. Process/Thread Scheduling\n");
        printf("  2. Memory Management (paging)\n");
        printf("  3. File System (NTFS-style tree)\n");
        printf("  4. Synchronization (mutex / semaphore)\n");
        printf("  0. Exit\n  Choice: ");
        if (scanf("%d", &choice) != 1) break;

        int sub;
        switch (choice) {
            case 1:
                printf("\n  1. Demo  2. Custom\n  Choice: ");
                if (scanf("%d", &sub) != 1) break;
                if (sub == 1) scheduler_run_demo(); else scheduler_run_custom();
                break;
            case 2:
                printf("\n  1. Demo  2. Custom\n  Choice: ");
                if (scanf("%d", &sub) != 1) break;
                if (sub == 1) memory_run_demo(); else memory_run_custom();
                break;
            case 3:
                printf("\n  1. Demo  2. Interactive\n  Choice: ");
                if (scanf("%d", &sub) != 1) break;
                if (sub == 1) filesystem_run_demo(); else filesystem_run_interactive();
                break;
            case 4:
                printf("\n  1. Demo  2. Interactive\n  Choice: ");
                if (scanf("%d", &sub) != 1) break;
                if (sub == 1) sync_run_demo(); else sync_run_interactive();
                break;
        }
    } while (choice != 0);

    printf("\n  Exiting.\n");
    return 0;
}
