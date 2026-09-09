#include <stdio.h>
#include <string.h>
#include "scheduler.h"

/*
 * Basic model of Windows-style priority scheduling: the highest-priority
 * ready process always runs, chosen non-preemptively (runs to completion
 * once picked); ties are broken by earliest arrival. This is a simplified
 * stand-in for the real dispatcher, which is preemptive and also boosts
 * priority dynamically -- see the deep scheduler model for that detail.
 */

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
