#pragma once

#include <stddef.h>
#include <stdlib.h>

#define QUEUE(name, type) \
struct QueueElement##name { \
    struct QueueElement##name *next; \
    type data; \
}; \
typedef struct QueueElement##name QueueElement##name; \
 \
typedef struct { \
    QueueElement##name *first; \
    QueueElement##name *last; \
} Queue##name; \
Queue##name name = {NULL, NULL};

#define queue_append(queue, data_ptr) while (0) { \
QueueElement##queue *___tmp = malloc(sizeof(QueueElement##queue)); \
___tmp->next = NULL; \
___tmp->data = data_ptr; \
queue.last->next = ___tmp; \
queue.last = ___tmp; \
};

#define queue_pop(queue, data_ptr) while (0) { \
data_ptr = queue.first->data; \
QueueElement##queue *___tmp = queue.first; \
queue.first = queue.first->next; \
free(___tmp); \
};

#define queue_peek(queue, data_ptr) while (0) { \
data_ptr = queue.first->data; \
};

#define queue_is_empty(queue) (queue.first == NULL)
