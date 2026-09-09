#include <stdio.h>
#include "memory.h"

/*
 * Basic model of Windows demand paging: a virtual page is loaded into
 * physical memory the first time it's touched (a page fault); once memory
 * is full, the oldest-loaded page is evicted (FIFO). Real Windows uses a
 * more elaborate working-set/aging scheme -- see the deep VMM model, which
 * also implements LRU and Clock for comparison.
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
