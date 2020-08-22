//
// Created by Adam on 27/05/2020.
//

#ifndef CODE_PRIORITYQUEUE_H
#define CODE_PRIORITYQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

/* Define Constants */
#define INITIAL_HEAP_SIZE 4

/* Struct Typedefs */
typedef struct heap {
    int sizeOfHeap;
    int numberOfNodes;
    process ** heapArray;
} heap;

/* Function Prototypes */
void createHeap(heap * h);
void minHeapify(process ** data, int currentNode, int count);
void heapPush(heap * h, process * currentProcess);
process * popHeapRoot(heap * h);
void freeHeap(heap * pq);

#endif //CODE_PRIORITYQUEUE_H
