#include "lock_mgr.h"
#include "bank.h"
#include <pthread.h>

extern Bank bank;

bool transfer(int from_id, int to_id, int amount_centavos) {
    // 1. Prevent self-transfers (or handle as no-op)
    if (from_id == to_id) return true;

    // 2. DEADLOCK PREVENTION: Lock Ordering
    // Always acquire the lock for the smaller account ID first.
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    Account* acc_first = &bank.accounts[first];
    Account* acc_second = &bank.accounts[second];

    // Acquire exclusive writer locks
    pthread_rwlock_wrlock(&acc_first->lock);
    pthread_rwlock_wrlock(&acc_second->lock);

    // 3. Perform the Transaction Logic
    Account* from_acc = &bank.accounts[from_id];
    Account* to_acc = &bank.accounts[to_id];

    bool success = false;
    if (from_acc->balance_centavos >= amount_centavos) {
        from_acc->balance_centavos -= amount_centavos;
        to_acc->balance_centavos += amount_centavos;
        success = true;
    }

    // 4. Release locks in any order (usually reverse of acquisition)
    pthread_rwlock_unlock(&acc_second->lock);
    pthread_rwlock_unlock(&acc_first->lock);

    return success;
}