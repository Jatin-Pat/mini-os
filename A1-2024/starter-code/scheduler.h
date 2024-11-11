#ifndef SCHEDULER_H
#define SCHEDULER_H

int run_scheduler();
int sequential_policy();
int round_robin_policy(int max_timer);
int aging_policy();

#endif
