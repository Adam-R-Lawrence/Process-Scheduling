/*
 *         priorityQueue.c
 * Computer Systems (COMP30023) - Project 2
 * Adam Lawrence || arlawrence || 992684
 * arlawrence@student.unimelb.edu.au
 *
 * Min Heap Priority Queue for my Custom Scheduling Algorithm
 *
 */

#include <assert.h>
#include "priorityQueue.h"

//To initialize the heap
void createHeap(heap* h)
{
    h->numberOfNodes = 0;
    h->sizeOfHeap = INITIAL_HEAP_SIZE;
    h->heapArray = (process **) malloc(sizeof(process *) * INITIAL_HEAP_SIZE);
    assert(h->heapArray);

    for(int i = 0; i < INITIAL_HEAP_SIZE; i++) {
        h->heapArray[i] = NULL;
    }
}

//Re-arrange the Heap so the process with the smallest job time is at the root index
void minHeapify(process** data, int currentNode, int count) {
    int leftChild, rightChild, smallest;
    process* temp;
    leftChild = 2 * (currentNode) + 1;
    rightChild = leftChild + 1;
    smallest = currentNode;

    //Check which has the smaller Job Time
    if (leftChild <= count && data[leftChild]->jobTime < data[smallest]->jobTime) {
        smallest = leftChild;
    }
    if (rightChild <= count && data[rightChild]->jobTime < data[smallest]->jobTime) {
        smallest = rightChild;
    }

    //If the Job Times are the same, then base the ordering on the process ID
    if (leftChild <= count && data[leftChild]->jobTime == data[smallest]->jobTime && data[leftChild]->processID < data[smallest]->processID) {
        smallest = leftChild;
    }
    if (rightChild <= count && data[rightChild]->jobTime == data[smallest]->jobTime && data[leftChild]->processID < data[smallest]->processID) {
        smallest = rightChild;
    }

    if(smallest != currentNode) {
        temp = data[currentNode];
        data[currentNode] = data[smallest];
        data[smallest] = temp;
        minHeapify(data, smallest, count);
    }

}

//Find where to place the node within the heap
void heapPush(heap* h, process * currentProcess)
{

    int currentIndex, parentNode;

    // Check to see if the Heap size needs to be expanded
    if (h->numberOfNodes == h->sizeOfHeap)
    {
        h->sizeOfHeap += 1;
        h->heapArray = realloc(h->heapArray, sizeof(process) * h->sizeOfHeap);
        assert(h->heapArray);
    }

    if(h->numberOfNodes == 0){
        h->heapArray[0] = currentProcess;
        h->numberOfNodes++;
        return;
    }


    // Find out where to put the Node
    currentIndex = h->numberOfNodes++;

    for(; currentIndex; currentIndex = parentNode)
    {

        parentNode = (currentIndex - 1) / 2;

        if (h->heapArray[parentNode]->jobTime <= currentProcess->jobTime) {
            break;
        }

        h->heapArray[currentIndex] = h->heapArray[parentNode];
    }

    h->heapArray[currentIndex] = currentProcess;
}

//Pop and return the root of the Heap (The process with the minimum remaining time)
process* popHeapRoot(heap* h)
{
    process * rootProcess;
    process* temp = h->heapArray[--h->numberOfNodes];

    //Shrink the size of the heap
    if ((h->numberOfNodes <= (h->sizeOfHeap + 2)) && (h->sizeOfHeap > INITIAL_HEAP_SIZE))
    {
        h->sizeOfHeap -= 1;
        h->heapArray = realloc(h->heapArray, sizeof(process) * h->sizeOfHeap);
        assert(h->heapArray);
    }

    rootProcess = h->heapArray[0];
    h->heapArray[0] = temp;
    if(temp == rootProcess){
        h->heapArray[0] = NULL;
    }

    //Fix the Heap
    minHeapify(h->heapArray, 0, h->numberOfNodes);

    return rootProcess;
}

//Free the heap from memory
void freeHeap(struct heap* h) {
    while(h->numberOfNodes != 0) {
        process * n = popHeapRoot(h);
        free(n);
    }
}
