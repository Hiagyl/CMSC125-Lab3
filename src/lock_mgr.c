#include "lock_mgr.h"
#include "bank.h"
#include <pthread.h>

extern Bank bank;

bool transfer(int from_id, int to_id, int amount_centavos) {
    // If transferring to the same account, it's a no-op or error
    if (from_id == to_id) return true;

    // 1. Determine lock order (Always lock the smaller ID first)
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    Account* acc_first = &bank.accounts[first];
    Account* acc_second = &bank.accounts[second];

    // 2. Acquire locks in the strictly defined order
    pthread_rwlock_wrlock(&acc_first->lock);
    pthread_rwlock_wrlock(&acc_second->lock);

    // 3. Perform the business logic
    Account* from_acc = &bank.accounts[from_id];
    Account* to_acc = &bank.accounts[to_id];

    if (from_acc->balance_centavos < amount_centavos) {
        // Release in any order, but usually reverse of acquisition
        pthread_rwlock_unlock(&acc_second->lock);
        pthread_rwlock_unlock(&acc_first->lock);
        return false; // Insufficient funds
    }

    from_acc->balance_centavos -= amount_centavos;
    to_acc->balance_centavos += amount_centavos;

    // 4. Release locks
    pthread_rwlock_unlock(&acc_second->lock);
    pthread_rwlock_unlock(&acc_first->lock);

    return true;
}