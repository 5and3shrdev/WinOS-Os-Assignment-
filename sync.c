#include <stdio.h>
#include <string.h>
#include "sync.h"

/*
 * Basic model of two Windows synchronization objects (accessed through
 * handles in the real OS): a Mutex, which only one thread may own at a
 * time, and a Semaphore, which lets up to N threads through at once.
 * Both maintain a simple FIFO wait queue for blocked threads.
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
