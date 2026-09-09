#include <stdio.h>
#include "scheduler.h"
#include "memory.h"
#include "filesystem.h"
#include "sync.h"

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
