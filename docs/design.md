# Design Documentation: OS Lab 3 Banking System

This document outlines the architectural decisions and synchronization strategies implemented for the concurrent banking system, supported by operating systems research and theoretical principles.

---

## 1. Deadlock Strategy Choice

### Strategy: Deadlock Prevention (Resource Hierarchy)
I chose **Deadlock Prevention** using **Lock Ordering** (also known as Hierarchical Locking).

### Justification
In a high-frequency banking simulation, the overhead of maintaining a "Wait-For-Graph" and running cycle-detection algorithms is computationally expensive. According to **Silberschatz et al. (2018)** in *Operating System Concepts*, prevention strategies are often preferred in systems where the cost of aborting and restarting transactions is high.

### Proof of Elimination (Breaking Coffman Conditions)
By implementing lock ordering in `lock_mgr.c`, I have effectively eliminated the **Circular Wait** condition—one of the four necessary conditions for deadlock identified by **E.G. Coffman (1971)**.

* **The Implementation:** Every transaction involving multiple accounts (e.g., `TRANSFER`) acquires locks in a strict linear order based on the account's numeric ID.
* **The Proof:** Let $L(n)$ be the lock for account $n$. By enforcing that $L(i)$ must be acquired before $L(j)$ if $i < j$, we establish a **partial ordering**. Since a cycle in a directed graph requires at least one edge $(v_j, v_i)$ where $j > i$, our ordering rule makes such an edge impossible.
* **Research Context:** This technique is a standard implementation of the **Resource Hierarchy Postulate**, which guarantees that a circular wait cannot occur as long as all processes request resources in a predefined order.

---

## 2. Buffer Pool Integration

### Loading and Unloading
* **Loading:** Accounts are loaded into the buffer pool via `load_account()` immediately before an operation.
* **Unloading:** Accounts are released via `unload_account()` immediately upon completion of the operation to maximize slot availability.

### Handling a Full Pool
The system utilizes **Counting Semaphores** (`empty_slots` and `full_slots`) to manage the 5-slot capacity.
* If the pool is full, `sem_wait(&pool->empty_slots)` transitions the thread to a **Blocked** state.
* **Justification:** This implements a **Blocking Producer-Consumer** pattern. Research by **Dijkstra (1965)** on cooperating sequential processes proved that semaphores provide a robust mechanism for synchronization and resource throttling without the need for busy-waiting (spinning), which saves CPU cycles.

---

## 3. Reader-Writer Lock Performance

### Justification for `pthread_rwlock_t`
In banking workloads, read-only queries (like `BALANCE` checks) are more frequent than updates. Standard mutexes suffer from **serialization bottlenecks**. 

### Benchmark Observations
* **Best Workload:** The `rwlock` implementation shows the largest performance gain on **Read-Heavy Traces**.
* **Reasoning:** Unlike `pthread_mutex_t`, which is strictly exclusive, `pthread_rwlock_t` allows for **Concurrent Read Access**. 
* **Research Insight:** Studies on *Scalability of Synchronization Primitives* (McKenney, 2005) demonstrate that RW-locks significantly reduce "cache-line bouncing" in multi-core systems by allowing shared ownership. However, it is noted that these locks must be used carefully to avoid **Writer Starvation**, where a constant stream of readers prevents updates.

---

## 4. Timer Thread Design

### Necessity of a Separate Thread
A separate timer thread is mandatory to act as the system's **Logical Clock** (Lamport, 1978). It decouples the "simulation time" from the wall-clock execution speed of the host CPU.

### Sequential Failure
If the timer were removed and operations were processed sequentially:
1. **Loss of Concurrency:** The system would effectively become a single-threaded batch processor, rendering the `pthread` implementation moot.
2. **Determinism:** The `start_tick` logic would fail to simulate **temporal overlaps**.

### Enabling True Concurrency Testing
The timer thread utilizes **Condition Variables** (`pthread_cond_broadcast`) to synchronize multiple worker threads. By forcing several transactions to wake up at the exact same `global_tick`, we create high **Resource Contention**. This is the only way to empirically verify that the lock ordering and buffer pool logic can handle race conditions—a core requirement for any robust concurrent system as discussed in *The Art of Multiprocessor Programming* (Herlihy & Shavit, 2008).

---

## References
* Arpaci-Dusseau, R. H., & Arpaci-Dusseau, A. C. (2018). *Operating Systems: Three Easy Pieces*.
* Coffman, E. G., Elphick, M., & Shoshani, A. (1971). "System Deadlocks". *ACM Computing Surveys*.
* Herlihy, M., & Shavit, N. (2008). *The Art of Multiprocessor Programming*.
* Silberschatz, A., Galvin, P. B., & Gagne, G. (2018). *Operating System Concepts*.