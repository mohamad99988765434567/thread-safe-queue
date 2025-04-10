#include <threads.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdatomic.h>

///////////////////////////////////////////
// Function Declarations
void initQueue(void);
void destroyQueue(void);
void enqueue(void *data);
void *dequeue(void);
bool tryDequeue(void **data);
size_t visited(void);

//////////////////////////////////
// Node structure for the queue
typedef struct Node {
    void *data;
    struct Node *next;
} Node;

// Condition node structure for FIFO wakeup order
typedef struct cndNode {
    cnd_t condition;
    struct cndNode *next;
} cndNode;

// Waiting list to enforce FIFO thread wake-up order
typedef struct waitList {
    cndNode *first;
    cndNode *last;
    int size;
} waitList;

// Queue structure
typedef struct {
    Node *head;
    Node *tail;
    mtx_t mutex;
    cnd_t not_empty;
    waitList wait_list;
} Queue;

// Global queue and atomic counters
Queue queue;
atomic_size_t visited_count;

//////////////////////////////////
// Initialize the queue
void initQueue(void) {
    queue.head = queue.tail = NULL;
    atomic_store(&visited_count, 0);
    queue.wait_list.first = queue.wait_list.last = NULL;
    queue.wait_list.size = 0;
    mtx_init(&queue.mutex, mtx_plain);
}

// Destroy the queue and free resources
void destroyQueue(void) {
    mtx_lock(&queue.mutex);

    while (queue.head) {
        Node *temp = queue.head;
        queue.head = queue.head->next;
        free(temp);
    }
    queue.tail = NULL;

    while (queue.wait_list.first) {
        cndNode *temp = queue.wait_list.first;
        queue.wait_list.first = queue.wait_list.first->next;
        cnd_destroy(&temp->condition);
        free(temp);
    }
    queue.wait_list.last = NULL;
    queue.wait_list.size = 0;

    mtx_unlock(&queue.mutex);
    mtx_destroy(&queue.mutex);
}

// Add an item to the queue
void enqueue(void *item) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) return;

    newNode->data = item;
    newNode->next = NULL;

    mtx_lock(&queue.mutex);

    if (queue.tail) {
        queue.tail->next = newNode;
    } else {
        queue.head = newNode;
    }
    queue.tail = newNode;

    if (queue.wait_list.size > 0) {
        cnd_signal(&queue.wait_list.first->condition);
    }
    
    mtx_unlock(&queue.mutex);
}

// Remove an item from the queue (blocks if empty) with strict FIFO wake-up order
void *dequeue(void) {
    void *data = NULL;
    cndNode *removed_cnd;
    Node *removed_node;

    mtx_lock(&queue.mutex);

    while (queue.head == NULL) {
        cndNode *cnd_node = (cndNode *)malloc(sizeof(cndNode));
        cnd_init(&cnd_node->condition);
        cnd_node->next = NULL;

        if (queue.wait_list.last) {
            queue.wait_list.last->next = cnd_node;
        } else {
            queue.wait_list.first = cnd_node;
        }
        queue.wait_list.last = cnd_node;
        queue.wait_list.size++;

        cnd_wait(&cnd_node->condition, &queue.mutex);

        removed_cnd = queue.wait_list.first;
        queue.wait_list.first = queue.wait_list.first->next;
        queue.wait_list.size--;
        if (queue.wait_list.size == 0) {
            queue.wait_list.last = NULL;
        }
        free(removed_cnd);
    }

    removed_node = queue.head;
    data = removed_node->data;
    queue.head = queue.head->next;
    if (!queue.head) {
        queue.tail = NULL;
    }
    free(removed_node);
    atomic_fetch_add(&visited_count, 1);

    if (queue.wait_list.size > 0 && queue.head) {
        cnd_signal(&queue.wait_list.first->condition);
    }
    
    mtx_unlock(&queue.mutex);
    return data;
}

// Try to remove an item from the queue (non-blocking)
bool tryDequeue(void **item) {
    mtx_lock(&queue.mutex);

    if (queue.head == NULL) {
        mtx_unlock(&queue.mutex);
        return false;
    }

    Node *node = queue.head;
    queue.head = node->next;
    if (!queue.head) {
        queue.tail = NULL;
    }

    atomic_fetch_add(&visited_count, 1);
    *item = node->data;
    free(node);

    mtx_unlock(&queue.mutex);
    return true;
}

// Return the number of visited items (lock-free)
size_t visited(void) {
    return atomic_load(&visited_count);
}
