#include "bank.h"
#include "timer.h"
#include <stdio.h>

Bank bank; // Global bank instance

int main() {
    // 1. Initialize Bank and Account Locks
    bank.num_accounts = MAX_ACCOUNTS;
    for(int i = 0; i < MAX_ACCOUNTS; i++) {
        bank.accounts[i].account_id = i;
        bank.accounts[i].balance_centavos = 0;
        pthread_rwlock_init(&bank.accounts[i].lock, NULL);
    }
    pthread_mutex_init(&bank.bank_lock, NULL);

    // 2. Start the Timer Thread
    pthread_t timer_tid;
    pthread_create(&timer_tid, NULL, timer_thread, NULL);

    // 3. (To be implemented) Load accounts and trace files
    
    printf("Bank system initialized and timer started.\n");
    
    // Clean up would happen here after simulation ends
    return 0;
}