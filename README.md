# c11-concurrent-queue

A lock-based FIFO queue implementation in C11 with thread safety, blocking dequeue, non-blocking tryDequeue, and strict FIFO wake-up semantics.

This is a basic implementation of a concurrent FIFO queue using the C11 standard threading library (<threads.h>). It provides support for enqueue, blocking dequeue, non-blocking dequeue, and tracking the number of visited (dequeued) items. Sleeping threads are woken in the same order they went to sleep, maintaining FIFO behavior.

## Features

- Thread-safe enqueue and dequeue
- FIFO thread wake-up ordering
- Lock-free read of the visited counter using C11 atomic operations
- Built using standard C11 threads and synchronization primitives (no pthreads)

## Functions

- `void initQueue(void)`  
  Initializes the queue. Must be called before using it.

- `void destroyQueue(void)`  
  Cleans up the queue. No other queue operations should run while this is called.

- `void enqueue(void *data)`  
  Adds a new item to the end of the queue.

- `void *dequeue(void)`  
  Removes and returns the front item. Blocks if the queue is empty.

- `bool tryDequeue(void **data)`  
  Attempts to remove an item without blocking. Returns false if the queue is empty.

- `size_t visited(void)`  
  Returns the total number of items that were dequeued. This call is lock-free.

## Compilation
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 -pthread -c queue.


## Notes

- Assumes `malloc()` succeeds.
- No pthreads used — only standard `<threads.h>` functionality.
- Wakes up only one thread per item, in FIFO order.
- `visited()` may be slightly off under concurrency but is exact when no concurrent access occurs.
