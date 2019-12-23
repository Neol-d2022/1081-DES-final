#include <stdlib.h>
#include <string.h>

#include "heapqueue.h"

static int HeapQueueIncreaseCapacity(HeapQueue_t *queue)
{
    void **array;
    unsigned int capacity = (queue->capacity) << 1;
    
    array = (void **)realloc(queue->array, sizeof(*array) * capacity);
    if (!array)
        return -1;
    else
    {
        queue->array = array;
        queue->capacity = capacity;
        return 0;
    }
}

static int HeapQueueDecreaseCapacity(HeapQueue_t *queue)
{
    void **array;
    unsigned int capacity = (queue->capacity) >> 1;
    
    array = (void **)realloc(queue->array, sizeof(*array) * capacity);
    if (!array)
        return -1;
    else
    {
        queue->array = array;
        queue->capacity = capacity;
        return 0;
    }
}

static inline unsigned int getParent(unsigned int index)
{
    if (!index)
        return (unsigned int)-1;
    else
        return (index - 1) >> 1;
}

static void HeapQueueRebuildOnInsert(HeapQueue_t *queue, unsigned int index)
{
    void *t;
    unsigned int parent;
    int r;
    
    parent = getParent(index);
    if (parent < queue->length)
    {
        r = (queue->comparer)((queue->array)[parent], (queue->array)[index]);
        if (r > 0)
        {
            t = (queue->array)[parent];
            (queue->array)[parent] = (queue->array)[index];
            (queue->array)[index] = t;
            
            HeapQueueRebuildOnInsert(queue, parent);
        }
    }
}

static inline unsigned int getLeftChild(unsigned int index)
{
    return (index << 1) + 1;
}

static void HeapQueueRebuildOnRetrieve(HeapQueue_t *queue, unsigned int index)
{
    void *t;
    unsigned int child[2], min;
    int r;
    
    child[0] = getLeftChild(index);
    child[1] = child[0] + 1;
    
    if (child[0] < queue->length)
    {
        r = (queue->comparer)((queue->array)[index], (queue->array)[child[0]]);
        if (r < 0)
        {
            if (child[1] < queue->length)
            {
                r = (queue->comparer)((queue->array)[index], (queue->array)[child[1]]);
                if (r < 0)
                    return;
            }
            else
                return;
        }
    }
    else
        return;
    
    min = child[0];
    if (child[1] < queue->length)
    {
        r = (queue->comparer)((queue->array)[child[0]], (queue->array)[child[1]]);
        if (r > 0)
            min = child[1];
    }
    
    t = (queue->array)[index];
    (queue->array)[index] = (queue->array)[min];
    (queue->array)[min] = t;
    
    HeapQueueRebuildOnRetrieve(queue, min);
}

// ================================================================

int HeapQueueInit(HeapQueue_t *queue, HeapQueueComparer_t comparer)
{
    void **array;
    
    memset(queue, 0, sizeof(*queue));
    
    array = (void **)malloc(sizeof(*array) * HEAP_QUEUE_INITIAL_CAPACITY);
    if (!array)
        return -1;
    
    queue->array = array;
    queue->capacity = HEAP_QUEUE_INITIAL_CAPACITY;
    queue->length = 0;
    queue->comparer = comparer;
    
    return 0;
}

int HeapQueueDestroy(HeapQueue_t *queue)
{
    free(queue->array);
    
    return 0;
}

int HeapQueueInsert(HeapQueue_t *queue, void *element)
{
    unsigned int insertIndex;
    
    while (queue->length >= queue->capacity)
        if (HeapQueueIncreaseCapacity(queue))
            return -1;
    
    (queue->array)[(insertIndex = (queue->length)++)] = element;
    HeapQueueRebuildOnInsert(queue, insertIndex);
    
    return 0;
}

int HeapQueueRetrieve(HeapQueue_t *queue, void **element)
{
    int ret;
    void *e;
    
    if (!(queue->length))
        return -1;
    
    e = (queue->array)[0];
    (queue->array)[0] = (queue->array)[--(queue->length)];
    HeapQueueRebuildOnRetrieve(queue, 0);
    
    ret = 0;
    while ((queue->capacity > HEAP_QUEUE_INITIAL_CAPACITY) && ((queue->capacity >> 2) > queue->length))
        if (HeapQueueDecreaseCapacity(queue))
            ret = 1;
    *element = e;
    
    return ret;
}

unsigned int HeapQueueLength(const HeapQueue_t *queue)
{
    return queue->length;
}