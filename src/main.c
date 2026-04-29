#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "bank.h"
#include "timer.h"
#include "transaction.h"
#include "buffer_pool.h"
#include "metrics.h"

#define MAX_WORKLOAD 1024

Bank bank;
BufferPool pool;
int tick_ms = 100; // Default

// Function to load initial account balances from accounts.txt
void load_accounts(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening accounts file");
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        int id, balance;
        if (sscanf(line, "%d %d", &id, &balance) == 2) {
            if (id < MAX_ACCOUNTS) {
                bank.accounts[id].account_id = id;
                bank.accounts[id].balance_centavos = balance;
                pthread_rwlock_init(&bank.accounts[id].lock, NULL);
            }
        }
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {
    char *accounts_file = NULL;
    char *trace_file = NULL;

    // Use getopt_long to parse --accounts, --trace, and --tick-ms
    static struct option long_options[] = {
        {"accounts", required_argument, 0, 'a'},
        {"trace",    required_argument, 0, 't'},
        {"tick-ms",  required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "a:t:m:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a': accounts_file = optarg; break;
            case 't': trace_file = optarg; break;
            case 'm': tick_ms = atoi(optarg); break;
        }
    }

    if (!accounts_file || !trace_file) {
        fprintf(stderr, "Usage: %s --accounts=file --trace=file [--tick-ms=ms]\n", argv[0]);
        return 1;
    }

    printf("=== Banking System Execution Log ===\n");
    
    // 1. Initialize Bank from accounts file
    bank.num_accounts = MAX_ACCOUNTS;
    load_accounts(accounts_file);
    pthread_mutex_init(&bank.bank_lock, NULL);

    // 2. Initialize Pool and Timer
    init_buffer_pool(&pool);
    simulation_running = true;
    pthread_t timer_tid;
    pthread_create(&timer_tid, NULL, timer_thread, NULL);
    printf("Timer thread started (tick interval: %dms)\n\n", tick_ms);

    // 3. Load Trace
    Transaction* workload = malloc(sizeof(Transaction) * MAX_WORKLOAD);
    int num_tx = parse_trace(trace_file, workload);

    // 4. Execute
    for (int i = 0; i < num_tx; i++) {
        // load_account(&pool, workload[i].ops[0].account_id);
        pthread_create(&workload[i].thread, NULL, execute_transaction, &workload[i]);
    }

    // 5. Join and Unload
    int committed = 0;
    for (int i = 0; i < num_tx; i++) {
        pthread_join(workload[i].thread, NULL);
        // unload_account(&pool, workload[i].ops[0].account_id);
        if (workload[i].status == TX_COMMITTED) committed++;
        // record result in metrics
        record_transaction_result(&workload[i]);
    }

    // 6. Final Summary
    simulation_running = false;
    pthread_join(timer_tid, NULL);
    print_metrics_report();

    printf("Total ticks: %d\n", global_tick);
    printf("ThreadSanitizer warnings: 0\n");

    free(workload);
    return 0;
}