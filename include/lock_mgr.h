#ifndef LOCK_MGR_H
#define LOCK_MGR_H

#include <stdbool.h>

// Strategy A: Deadlock Prevention via Lock Ordering
// Acquires locks in ascending order of account ID to break circular wait
bool transfer(int tx_id, int from_id, int to_id, int amount_centavos);

#endif