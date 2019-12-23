#ifndef _LINKED_QUEUE_H_
#define _LINKED_QUEUE_H_

typedef struct LinkedQueueElement_struct_t {
    void *element;
    struct LinkedQueueElement_struct_t *next;
} LinkedQueueElement_t;

typedef struct {
    LinkedQueueElement_t *head;
    LinkedQueueElement_t **last;
    unsigned int length;
} LinkedQueue_t;

int LinkedQueueInit(LinkedQueue_t *queue);
int LinkedQueueDestroy(LinkedQueue_t *queue);

int LinkedQueueInsert(LinkedQueue_t *queue, void *element);
int LinkedQueueRetrieve(LinkedQueue_t *queue, void **element);

unsigned int LinkedQueueLength(const LinkedQueue_t *queue);

#endif
