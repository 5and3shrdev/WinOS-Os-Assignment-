#ifndef SCHEDULER_H
#define SCHEDULER_H

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

void scheduler_run_demo(void);
void scheduler_run_custom(void);

#endif
