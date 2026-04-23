#include "metrics.h"
#include <stdio.h>

static int total_committed = 0;
static int total_aborted = 0;
static int total_wait_ticks = 0;

void record_transaction_result(Transaction* tx) {
    if (tx->status == TX_COMMITTED) {
        total_committed++;
    } else {
        total_aborted++;
    }
    total_wait_ticks += tx->wait_ticks;
}

void print_metrics_report() {
    printf("\n=== Summary ===\n");
    printf("Total transactions: %d\n", total_committed + total_aborted);
    printf("Committed: %d\n", total_committed);
    printf("Aborted:   %d\n", total_aborted);
    printf("Total Lock Wait Time:   %d ticks\n", total_wait_ticks);
    if (total_committed > 0) {
        printf("Avg Wait Per Tx:        %.2f ticks\n", (float)total_wait_ticks / (total_committed + total_aborted));
    }
}