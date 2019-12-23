#ifndef _HEAP_QUEUE_H_
#define _HEAP_QUEUE_H_

#define HEAP_QUEUE_INITIAL_CAPACITY 8

typedef int (*HeapQueueComparer_t)(const void *, const void *);

typedef struct {
    void **array;
    HeapQueueComparer_t comparer;
    unsigned int capacity;
    unsigned int length;
} HeapQueue_t;

int HeapQueueInit(HeapQueue_t *queue, HeapQueueComparer_t comparer);
int HeapQueueDestroy(HeapQueue_t *queue);

int HeapQueueInsert(HeapQueue_t *queue, void *element);
int HeapQueueRetrieve(HeapQueue_t *queue, void **element);

unsigned int HeapQueueLength(const HeapQueue_t *queue);

#endif
