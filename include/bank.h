#ifndef BANK_H
#define BANK_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 100

typedef struct {
    int account_id;
    int balance_centavos;
    pthread_rwlock_t lock; // Per-account reader-writer lock
} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;
    pthread_mutex_t bank_lock; // Protects bank-wide metadata

    // Conservation Tracking
    long long initial_sum;
    long long total_deposited;
    long long total_withdrawn;
} Bank;

int get_balance(int account_id);
void deposit(int account_id, int amount_centavos);
bool withdraw(int account_id, int amount_centavos);

#endif