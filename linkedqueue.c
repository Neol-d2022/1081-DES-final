#include <stdlib.h>
#include <string.h>

#include "linkedqueue.h"

// ================================================================

int LinkedQueueInit(LinkedQueue_t *queue)
{
    memset(queue, 0, sizeof(*queue));
    queue->last = &(queue->head);
    
    return 0;
}

int LinkedQueueDestroy(LinkedQueue_t *queue)
{
    LinkedQueueElement_t *cur = queue->head, *next;
    
    while (cur)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
    
    return 0;
}

int LinkedQueueInsert(LinkedQueue_t *queue, void *element)
{
    LinkedQueueElement_t *n;
    
    n = (LinkedQueueElement_t *)malloc(sizeof(*n));
    if (!n)
        return -1;
    
    n->element = element;
    n->next = *(queue->last);
    
    *(queue->last) = n;
    queue->last = &(n->next);
    queue->length++;
    
    return 0;
}

int LinkedQueueRetrieve(LinkedQueue_t *queue, void **element)
{
    LinkedQueueElement_t *n;
    
    if (!(queue->length))
        return -1;
    
    n = queue->head;
    queue->head = queue->head->next;
    if(--(queue->length) == 0)
        queue->last = &(queue->head);
    
    *element = n->element;
    free(n);
    
    return 0;
}

unsigned int LinkedQueueLength(const LinkedQueue_t *queue)
{
    return queue->length;
}
