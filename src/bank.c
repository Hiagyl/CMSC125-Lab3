#include "bank.h"
#include <stdio.h>
#include <stdbool.h>

extern Bank bank; // Defined in main.c

int get_balance(int account_id) {
    Account* acc = &bank.accounts[account_id];
    pthread_rwlock_rdlock(&acc->lock); // Multiple threads can read at once
    int balance = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);
    return balance;
}

void deposit(int account_id, int amount_centavos) {
    // Update account balance
    pthread_rwlock_wrlock(&bank.accounts[account_id].lock);
    bank.accounts[account_id].balance_centavos += amount_centavos;
    pthread_rwlock_unlock(&bank.accounts[account_id].lock);

    // Track external inflow (Protected by bank_lock)
    pthread_mutex_lock(&bank.bank_lock);
    bank.total_deposited += amount_centavos;
    pthread_mutex_unlock(&bank.bank_lock);
}


bool withdraw(int account_id, int amount_centavos) {
    bool success = false;
    pthread_rwlock_wrlock(&bank.accounts[account_id].lock);
    if (bank.accounts[account_id].balance_centavos >= amount_centavos) {
        bank.accounts[account_id].balance_centavos -= amount_centavos;
        success = true;
    }
    pthread_rwlock_unlock(&bank.accounts[account_id].lock);

    if (success) {
        // Track external outflow
        pthread_mutex_lock(&bank.bank_lock);
        bank.total_withdrawn += amount_centavos;
        pthread_mutex_unlock(&bank.bank_lock);
    }
    return success;
}