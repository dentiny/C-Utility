#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <atomic>
#include "queue.h"

#define DEQUEUE_FAIL -99

// this is not a type you need to think about yourself
// I use it to do the appropiate conversions in the cas function
union union_type {
    pointer_t parts;
    unsigned __int128 as_int128;
};

bool operator==(const pointer_t& lhs, const pointer_t& rhs) {
    return lhs.counter == rhs.counter && lhs.pointer == rhs.pointer;
}

bool cas(pointer_t *src,
         pointer_t expected,
         pointer_t value)
{
    union union_type uexp = { .parts = expected };
    union union_type uvalue = { .parts = value };
    return __sync_bool_compare_and_swap((__int128*) src,
                                        uexp.as_int128,
                                        uvalue.as_int128);
}

void initialize(queue_t *new_queue) {
    node_t *node = (node_t*) malloc(sizeof(node_t));
    assert(node != NULL);
    node->next.pointer = NULL;
    node->next.counter = 0;
    new_queue->head.pointer = new_queue->tail.pointer = node;
    new_queue->head.counter = new_queue->tail.counter = 0;
}

/* Inserts an item in the queue */
void enqueue(queue_t *queue, int value) {
    node_t *node = (node_t*) malloc(sizeof(node_t));
    assert(node != NULL);
    node->value = value;
    node->next.pointer = NULL;

    pointer_t tail;
    pointer_t next;
    while (true) {
        tail = queue->tail;
        next = tail.pointer->next;
        if (tail == queue->tail) {
            if (next.pointer == NULL) {
                if (cas(&tail.pointer->next, next, { node, next.counter + 1 })) {
                    break;
                }
            } else {
                cas(&queue->tail, tail, { next.pointer, tail.counter + 1 });
            }
        }
    }
    cas(&queue->tail, tail, { node, tail.counter + 1 });
}

/**
Removes an item from the queue and returns its value, or DEQUEUE_FAIL
if the list is empty.

 */
int dequeue(queue_t *queue) {
    pointer_t head;
    pointer_t tail;
    pointer_t next;
    int val;

    while (true) {
        head = queue->head;
        tail = queue->tail;
        next = head.pointer->next;
        if (head == queue->head) {
            if (head.pointer == tail.pointer) {
                if (next.pointer == NULL) {
                    return DEQUEUE_FAIL;
                }
                cas(&queue->tail, tail, { next.pointer, tail.counter + 1 });
            }
            else {
                val = next.pointer->value;
                if (cas(&queue->head, head, { next.pointer, head.counter + 1 })) {
                    break;
                }
            }
        }
    }
    free(head.pointer);
    return val;
}

// frees resources associated with the queue
// not intented to be called in a concurrent context
void destroy(queue_t *queue) {
    while(queue->head.pointer != NULL) {
        node_t *to_free = queue->head.pointer;
        queue->head = to_free->next;
        free(to_free);
    }
}