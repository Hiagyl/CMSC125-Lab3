# OS Lab 3: Concurrent Banking System

A multi-threaded banking simulation demonstrating core Operating Systems concepts including thread synchronization, deadlock prevention, and resource-constrained buffer management.

---

## 1. Group Members
* **[First Member Name Placeholder]**
* **[Second Member Name Placeholder]**

---

## 2. Implemented Features

* **Multithreaded Transaction Processing:** Each transaction from the trace file is executed in its own dedicated thread.
* **Discrete Event Simulation:** A centralized **Timer Thread** manages a `global_tick` system to synchronize transaction start times.
* **Deadlock Prevention:** Implements a strict **Resource Hierarchy (Lock Ordering)** to prevent circular wait conditions during transfers.
* **Buffer Pool Management:** A fixed-size (5-slot) buffer pool that throttles concurrent access to account records using **Counting Semaphores**.
* **Fine-Grained Synchronization:** Utilizes `pthread_rwlock_t` (Reader-Writer locks) for individual accounts to allow concurrent balance inquiries while ensuring exclusive access for updates.
* **Dynamic Trace Parsing:** Support for custom workload files including `DEPOSIT`, `WITHDRAW`, `TRANSFER`, and `BALANCE` operations.
* **Metrics Reporting:** Comprehensive summary at the end of execution showing committed vs. aborted transactions and average lock wait times.

---

## 3. Compilation and Usage

### Prerequisites
* `gcc` compiler
* `make` utility
* POSIX Threads library (`pthread`)

### Compilation
To compile the system, run the following command in the project root:

```bash
make

## 3. Usage

Run the executable with the required `--accounts` and `--trace` flags:
```bash
./bank_sim --accounts=accounts.txt --trace=trace.txt --tick-ms=100

**Arguments:**

* **--accounts:** Path to the file containing initial account balances.
* **--trace:** Path to the file containing the list of transactions to process.
* **--tick-ms (Optional):** The duration of a single simulation tick in milliseconds (default is 100ms).

---

### 4. Known Limitations

* **Fixed Buffer Pool Size:** The buffer pool size is hardcoded to 5 slots. If a single transaction requires more than 5 accounts simultaneously (not possible with current op types), the system would block indefinitely.
* **Account ID Range:** The system assumes account IDs are within the range of 0 to `MAX_ACCOUNTS - 1`. Non-integer or out-of-range IDs in the input files may cause undefined behavior.
* **Writer Starvation:** While `pthread_rwlock_t` increases performance for reads, a constant stream of `BALANCE` operations could theoretically "starve" a `TRANSFER` operation from acquiring a write lock.
* **Memory Overhead:** For every transaction in a trace, a new thread is spawned. For extremely large traces (thousands of simultaneous transactions), this may exceed the OS thread limit. A thread pool would be the preferred architectural improvement for larger workloads.

---

### 5. File Structure

* **main.c:** Entry point and orchestration logic.
* **bank.c:** Account data structures and core banking operations.
* **transaction.c:** Worker thread logic and operation routing.
* **lock_mgr.c:** Deadlock-safe transfer logic.
* **buffer_pool.c:** Semaphore-based resource management.
* **timer.c:** Simulation clock and thread notification.
* **utils.c:** Trace and account file parsing.
* **metrics.c:** Performance tracking and final reporting.