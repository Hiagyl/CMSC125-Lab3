#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>
#include <stdbool.h>
#define TICK_INTERVAL_MS 100 // Adjust based on lab requirements

// External declarations for global synchronization
extern volatile int global_tick;
extern pthread_mutex_t tick_lock;
extern pthread_cond_t tick_changed;
extern bool simulation_running;

void* timer_thread();
void wait_until_tick(int target_tick);

#endif