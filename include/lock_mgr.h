#ifndef LOCK_MGR_H
#define LOCK_MGR_H

#include <stdbool.h>

// Strategy A: Prevention through consistent ordering
bool transfer(int from_id, int to_id, int amount_centavos);

#endif