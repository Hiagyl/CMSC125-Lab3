#include "lock_mgr.h"
#include "bank.h"
#include <stdio.h>

extern Bank bank;

bool transfer(int tx_id, int from_id, int to_id, int amount_centavos) {
    // 1. Prevent self-transfers (or handle as no-op)
    if (from_id == to_id) return true;

    // 2. DEADLOCK PREVENTION: Lock Ordering
    // Always acquire the lock for the smaller account ID first.
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    // Log acquisition of the first lock
    printf("  T%d acquired lock on account %d\n", tx_id, first);
    pthread_rwlock_wrlock(&bank.accounts[first].lock);

    // Log the prevention/waiting for the second lock
    printf("  [DEADLOCK PREVENTED] Lock ordering: T%d waiting for account %d\n", tx_id, second);
    pthread_rwlock_wrlock(&bank.accounts[second].lock);
    printf("  T%d acquired lock on account %d\n", tx_id, second);

    // 3. Perform the Transaction Logic
    bool success = false;
    if (bank.accounts[from_id].balance_centavos >= amount_centavos) {
        bank.accounts[from_id].balance_centavos -= amount_centavos;
        bank.accounts[to_id].balance_centavos += amount_centavos;
        success = true;
    } else {
        printf("  T%d: TRANSFER failed (Insufficient Funds)\n", tx_id);
    }

    // 4. Release locks in any order (usually reverse of acquisition)
    pthread_rwlock_unlock(&bank.accounts[second].lock);
    pthread_rwlock_unlock(&bank.accounts[first].lock);
    return success;
}