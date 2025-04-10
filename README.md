# concurrent-fifo-c11-queue

A thread-safe concurrent FIFO queue in C11 with strict FIFO wake-up order for waiting threads. Built using <threads.h> with C11 standard threading primitives.

This project implements a concurrent, thread-safe FIFO queue in C using the C11 <threads.h> API. It supports multiple threads enqueuing and dequeuing elements while enforcing strict FIFO ordering for threads waiting on an empty queue. Developed as part of the Operating Systems course at Tel Aviv University.

The queue allows concurrent access and synchronization between threads without data races. Threads that call dequeue() on an empty queue are blocked and placed in a FIFO waiting list. When items are added, the oldest waiting thread is signaled first, ensuring fairness. A lock-free visited() counter keeps track of how many items were removed from the queue. This counter uses atomic operations and never takes a mutex.

Features:
- Thread-safe enqueue and dequeue operations
- FIFO wake-up order for sleeping threads
- Lock-free counter using C11 atomic operations
- Built using only C11 standard threading primitives (mtx_t, cnd_t, thrd_t)
- Clean synchronization and memory management
- Blocks only when necessary (no spurious wakeups)

Functions:
- void initQueue(void)  
  Initializes the queue. Must be called before use.

- void destroyQueue(void)  
  Frees all memory used by the queue. No other operations may be active.

- void enqueue(void *data)  
  Adds a new item to the end of the queue. Signals waiting threads if needed.

- void *dequeue(void)  
  Removes and returns the front item. Blocks if the queue is empty, and wakes threads in FIFO order.

- bool tryDequeue(void **data)  
  Tries to dequeue without blocking. Returns true on success, false if the queue is empty.

- size_t visited(void)  
  Returns the total number of dequeued items. This function is non-blocking and lock-free.

Compilation:
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 -pthread -c queue.c

Notes:
- Sleeping threads are woken strictly in FIFO order.
- No deadlocks or race conditions.
- visited() may be slightly inaccurate under high concurrency but is exact when no concurrent modifications are occurring.
- malloc() is assumed not to fail.

Structure:
- Custom Node structure for items.
- cndNode and waitList for FIFO condition variable handling.
- One global queue instance protected by mtx_t.
- Atomic counter for visited() tracking.

Example usage (conceptual):
initQueue();  
enqueue(some_pointer);  
void* data = dequeue();  
destroyQueue();

Requirements met:
- Blocking dequeue() with FIFO thread wake-up  
- Non-blocking tryDequeue() behavior  
- Lock-free visited() read  
- Clean synchronization using cnd_wait, cnd_signal, and mtx_lock  

This queue was implemented entirely with <threads.h> (C11) and without using pthreads or any non-standard libraries.
