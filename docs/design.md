# Design Documentation: Concurrent Banking System

This document outlines the architectural decisions and synchronization strategies implemented for the OS Lab 3 Banking System.

---

## 1. Deadlock Strategy Choice

### Strategy: Deadlock Prevention
For this implementation, I chose **Deadlock Prevention** using **Resource Hierarchy (Lock Ordering)**.

### Justification
Prevention was chosen over detection because it is computationally cheaper and provides a proactive guarantee of system liveness. In a high-frequency banking simulation, the overhead of constantly running a cycle-detection algorithm on a dependency graph would significantly degrade performance.

### Proof of Correctness (Breaking Coffman Conditions)
By implementing lock ordering in `lock_mgr.c`, I have effectively broken the **Circular Wait** condition. 
*   **The Logic:** Every transaction that involves two accounts (e.g., `TRANSFER`) is forced to acquire locks in a strict linear order—specifically, the account with the **numerically smaller ID** is always locked first.
*   **The Result:** Since all threads follow the same global hierarchy, it is mathematically impossible to form a cycle where Thread A holds Lock 1 and waits for Lock 2, while Thread B holds Lock 2 and waits for Lock 1.

---

## 2. Buffer Pool Integration

### Loading and Unloading
*   **Loading:** Accounts are loaded into the buffer pool immediately before an operation begins in `execute_transaction` using the `load_account` function.
*   **Unloading:** Accounts are unloaded as soon as the specific operation (or the entire transaction in case of an abort) is finished.

### Handling a Full Pool
The buffer pool utilizes **Semaphores** (`empty_slots` and `full_slots`) to manage concurrency.
*   If the pool is full, a transaction calling `load_account` will block on `sem_wait(&pool->empty_slots)`.
*   The thread remains asleep until an existing transaction finishes its operation and calls `unload_account`, which executes `sem_post(&pool->empty_slots)` to wake up the next waiting thread.

### Justification
This design ensures **Correctness** by limiting the number of active records in "memory," mimicking real-world database constraints. It improves **Performance** by allowing up to `BUFFER_POOL_SIZE` transactions to progress in parallel while naturally throttling the system to prevent resource exhaustion.

---

## 3. Reader-Writer Lock Performance

### Justification for `pthread_rwlock_t`
In banking systems, read-heavy operations (like `BALANCE` checks) are common. While a `pthread_mutex_t` is strictly exclusive, a `pthread_rwlock_t` allows multiple threads to hold a "read lock" simultaneously as long as no thread holds a "write lock".

### Benchmark Observations
*   **Best Workload:** The `rwlock` implementation shows the most significant improvement in **Read-Heavy Traces** (e.g., a trace where 80% of operations are `BALANCE`).
*   **Reasoning:** Under a standard mutex, five simultaneous `BALANCE` requests would be processed sequentially. With `rwlock`, all five can execute in parallel, reducing the total "ticks" required to complete the trace.

---

## 4. Timer Thread Design

### Necessity of a Separate Thread
A separate timer thread is necessary to decouple the "system time" from the "execution speed" of individual transactions. 
*   **The Timer's Role:** It maintains the `global_tick` and uses `pthread_cond_broadcast` to notify all waiting transaction threads that time has progressed.

### Sequential Failure
If the timer were removed and operations were processed sequentially:
1.  The system would no longer be a concurrent simulation.
2.  The `start_tick` logic would become meaningless, as every transaction would simply wait for the one before it to finish regardless of its scheduled start time.

### Enabling True Concurrency Testing
The timer thread allows us to schedule multiple transactions to start at the **exact same tick**. This creates intentional "race conditions" and lock contention, which is the only way to verify that our `lock_mgr.c` and `buffer_pool.c` synchronization logic actually works under pressure.