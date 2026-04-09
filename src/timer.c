#include "timer.h"
#include <unistd.h>
#include <stdbool.h>

volatile int global_tick = 0;
bool simulation_running = true;
pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tick_changed = PTHREAD_COND_INITIALIZER;

void* timer_thread(void* arg) {
    while (simulation_running) {
        // Sleep to simulate time passing
        usleep(TICK_INTERVAL_MS * 1000); 
        
        pthread_mutex_lock(&tick_lock);
        global_tick++;
        // Wake up transaction threads waiting for this specific tick
        pthread_cond_broadcast(&tick_changed); 
        pthread_mutex_unlock(&tick_lock);
    }
    return NULL;
}

void wait_until_tick(int target_tick) {
    pthread_mutex_lock(&tick_lock);
    while (global_tick < target_tick) {
        pthread_cond_wait(&tick_changed, &tick_lock);
    }
    pthread_mutex_unlock(&tick_lock);
}