#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "bank.h"
#include "timer.h"
#include "transaction.h"
#include "buffer_pool.h"

// Number of transactions to run for this simulation
#define NUM_TRANSACTIONS 3 

Bank bank;
BufferPool pool;

// Dummy helper to simulate reading a trace file
void load_dummy_workload(Transaction* tx_list) {
    // Transaction 0: Deposit 1000 into Account 1 at Tick 2
    tx_list[0].tx_id = 0;
    tx_list[0].start_tick = 2;
    tx_list[0].num_ops = 1;
    tx_list[0].ops[0] = (Operation){OP_DEPOSIT, 1, 1000, -1};

    // Transaction 1: Transfer 500 from Account 1 to Account 2 at Tick 4
    tx_list[1].tx_id = 1;
    tx_list[1].start_tick = 4;
    tx_list[1].num_ops = 1;
    tx_list[1].ops[0] = (Operation){OP_TRANSFER, 1, 500, 2};

    // Transaction 2: Check Balance of Account 1 at Tick 6
    tx_list[2].tx_id = 2;
    tx_list[2].start_tick = 6;
    tx_list[2].num_ops = 1;
    tx_list[2].ops[0] = (Operation){OP_BALANCE, 1, 0, -1};
}

int main() {
    printf("--- Starting Concurrent Banking System ---\n");

    // 1. Initialize Bank and Account Locks
    bank.num_accounts = MAX_ACCOUNTS;
    for(int i = 0; i < MAX_ACCOUNTS; i++) {
        bank.accounts[i].account_id = i;
        bank.accounts[i].balance_centavos = 0;
        pthread_rwlock_init(&bank.accounts[i].lock, NULL);
    }
    pthread_mutex_init(&bank.bank_lock, NULL);

    // 2. Initialize Buffer Pool
    init_buffer_pool(&pool);

    // 3. Start the Timer Thread
    pthread_t timer_tid;
    simulation_running = true;
    pthread_create(&timer_tid, NULL, timer_thread, NULL);

    // 4. Load Workload (Trace Files)
    Transaction workload[NUM_TRANSACTIONS];
    load_dummy_workload(workload);

    // 5. Execute Transactions (The Concurrency Phase)
    // In a real system, you'd load accounts into the buffer pool before use
    // For this simulation, we spawn a thread for every transaction in the workload
    for (int i = 0; i < NUM_TRANSACTIONS; i++) {
        // Optional: Load accounts involved into buffer pool for "bounded" tracking
        load_account(&pool, workload[i].ops[0].account_id);
        
        pthread_create(&workload[i].thread, NULL, execute_transaction, &workload[i]);
    }

    // 6. Wait for all transactions to finish
    for (int i = 0; i < NUM_TRANSACTIONS; i++) {
        pthread_join(workload[i].thread, NULL);
        
        // Unload after thread is done
        unload_account(&pool, workload[i].ops[0].account_id);
        
        // Print results
        const char* status = (workload[i].status == TX_COMMITTED) ? "COMMITTED" : "ABORTED";
        printf("TX %d finished: %s (Wait Ticks: %d)\n", 
                workload[i].tx_id, status, workload[i].wait_ticks);
    }

    // 7. Shutdown and Cleanup
    printf("\nShutting down simulation...\n");
    simulation_running = false;
    pthread_join(timer_tid, NULL);

    // Destroy locks
    for(int i = 0; i < MAX_ACCOUNTS; i++) {
        pthread_rwlock_destroy(&bank.accounts[i].lock);
    }
    pthread_mutex_destroy(&bank.bank_lock);
    pthread_mutex_destroy(&tick_lock);
    pthread_cond_destroy(&tick_changed);

    printf("Cleanup complete. Final balance Account 1: %d centavos\n", get_balance(1));
    return 0;
}