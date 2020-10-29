#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define DEQUEUE_FAIL -99

struct node_t;

typedef struct {
    struct node_t *pointer;
    uint64_t counter;
} __attribute__ ((aligned (16))) pointer_t;

struct node_t {
    int value;
    pointer_t next;
};

typedef struct node_t node_t;

typedef struct {
    pointer_t head;
    pointer_t tail;
} queue_t;

// sets up and initializes a queue
// not intented to be called in a concurrent context
void initialize(queue_t *new_queue);

void enqueue(queue_t *queue, int value);

int dequeue(queue_t *queue);

// frees resources associated with the queue
// not intented to be called in a concurrent context
void destroy(queue_t *queue);