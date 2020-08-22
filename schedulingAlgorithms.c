/*
 *         schedulingAlgorithms.c
 * Computer Systems (COMP30023) - Project 2
 * Adam Lawrence || arlawrence || 992684
 * arlawrence@student.unimelb.edu.au
 *
 */

#include "scheduler.h"
#include "schedulingAlgorithms.h"
#include "priorityQueue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

queueNode * pointerTopQueue = NULL;
queueNode * pointerBottomQueue = NULL;
queueNode * pointerTopProcess = NULL;
queueNode * pointerBottomProcess = NULL;
processID * pointerTopID = NULL;
processID * pointerBottomID = NULL;
extern int * memory;

//First-Come First-Serve Scheduling Algorithm
void firstComeFirstServe(int memorySize, char memoryAllocation[]) {
    int currentTime = 0;
    int processesCompleted = 0;
    int totalTurnaroundTime = 0;
    int numberOfProcesses = 0;

    double * throughput = malloc(sizeof(double));
    double * overhead = malloc(sizeof(double));
    int * memoryFrequency= malloc(sizeof(int) * memorySize);

    for(int i = 0; i < memorySize; i++) {
        memoryFrequency[i] = 0;
    }

    int throughputSize = 1;
    throughput[0] = 0;

    process * currentProcess;
    process * tempPtr = malloc(sizeof(process));

    //Enqueue the first process to be executed (Arrival time of 0 with the smalled process ID)
    popFromListOfProcesses(tempPtr);
    enqueueProcessToExecute(tempPtr->timeArrived,tempPtr->processID,tempPtr->memorySizeRequired,tempPtr->jobTime,tempPtr->jobTimeLeft);
    free(tempPtr);
    numberOfProcesses++;

    //Execute Processes that have arrived
    while(processesCompleted < numberOfProcesses) {

        //Get Process to be Executed
        currentProcess = malloc(sizeof(process));
        dequeueProcessToExecute(currentProcess);

        //Check if there has been a Time Jump
        if(currentTime < currentProcess->timeArrived){
            currentTime = currentProcess->timeArrived;
        }

        //Load the process onto memory if there is not unlimited memory
        if(strcmp(memoryAllocation,"u") != 0) {
            int loadTime;

            if(strcmp(memoryAllocation,"p") == 0) {
                loadTime = swappingX(currentProcess, memorySize, currentTime);
            } else if(strcmp(memoryAllocation,"v") == 0) {
                loadTime = virtualMemory(currentProcess, memorySize, currentTime);
            } else if(strcmp(memoryAllocation,"cm") == 0) {
                loadTime = mostFrequentlyUsed(currentProcess, memorySize, currentTime, memoryFrequency);
            }

            printRUNNING(currentTime, currentProcess->processID, currentProcess->jobTimeLeft, memorySize, loadTime);
            currentTime = currentTime + loadTime;

        }else {

            printRUNNINGWithUnlimited(currentTime, currentProcess->processID, currentProcess->jobTimeLeft);
        }

        //Update the current time of the simulation and set Job time left to 0 as FIFO is Non Preemptive
        currentTime = currentTime + currentProcess->jobTime;
        currentProcess->jobTimeLeft = 0;


        //See what processes have arrived since executing the current process and add them to the queue
        while(((pointerBottomProcess != NULL) && pointerBottomProcess->thisProcess.timeArrived <= currentTime)) {
            tempPtr = malloc(sizeof(process));

            popFromListOfProcesses(tempPtr);
            enqueueProcessToExecute(tempPtr->timeArrived,tempPtr->processID,tempPtr->memorySizeRequired,tempPtr->jobTime,tempPtr->jobTimeLeft);

            numberOfProcesses++;
            free(tempPtr);
        }


        //Check if we need to expand the Throughput array
        while(currentTime > (THROUGHPUT_INTERVAL_SIZE * throughputSize)) {
            throughputSize++;
            throughput = realloc(throughput,sizeof(double) * throughputSize);
            throughput[throughputSize-1] = 0;

        }

        throughput[throughputSize-1]++;

        totalTurnaroundTime = totalTurnaroundTime + (currentTime - currentProcess->timeArrived);;
        int turnaroundTime = currentTime - currentProcess->timeArrived;


        overhead[processesCompleted] = 1.0* turnaroundTime/currentProcess->jobTime;


        enqueueProcessID(currentProcess->processID);

        //Increment the amount of process completed
        processesCompleted++;

        //Evict from memory
        if(currentProcess->jobTimeLeft == 0) {
            if(strcasecmp(memoryAllocation,"u") != 0) {
                evictProcessFromMemory(memorySize, currentProcess->processID, currentTime);
            }
        }

        printFINISHED(currentTime,currentProcess->processID, numberOfProcesses - processesCompleted);

        //If there is no process left to be executed wait until one comes in (Time Jump)
        if(pointerBottomQueue == NULL && pointerBottomProcess != NULL) {
            tempPtr = malloc(sizeof(process));

            popFromListOfProcesses(tempPtr);
            enqueueProcessToExecute(tempPtr->timeArrived,tempPtr->processID,tempPtr->memorySizeRequired,tempPtr->jobTime,tempPtr->jobTimeLeft);

            numberOfProcesses++;
            free(tempPtr);
        }

        overhead = realloc(overhead, sizeof(double) * numberOfProcesses);

        free(currentProcess);
    }

    //Print the statistics of the simulation
    printStatistics(throughput,throughputSize,totalTurnaroundTime,overhead,numberOfProcesses,currentTime);

    free(overhead);
    free(throughput);
    free(memoryFrequency);
}

//Round Robin Scheduling Algorithm
void roundRobin(int memorySize,int quantum, char memoryAllocation[]) {
    int currentTime = 0;
    int processesCompleted = 0;
    int totalTurnaroundTime = 0;
    int numberOfProcesses = 0;
    double * throughput = malloc(sizeof(double));
    double * overhead = malloc(sizeof(double));
    int throughputSize = 1;
    throughput[0] = 0;
    process * currentProcess;
    int * executedProcessID = malloc(sizeof(int));
    int * memoryFrequency= malloc(sizeof(int) * memorySize);
    for(int i = 0; i < memorySize; i++) {
        memoryFrequency[i] = 0;
    }
    process  * tempPtr;
    tempPtr = malloc(sizeof(process));

    //Enqueue the first process to be executed (Arrival time of 0 with the smalled process ID)
    popFromListOfProcesses(tempPtr);
    enqueueProcessToExecute(tempPtr->timeArrived,tempPtr->processID,tempPtr->memorySizeRequired,tempPtr->jobTime,tempPtr->jobTimeLeft);
    free(tempPtr);
    numberOfProcesses++;

    //Execute Processes that have arrived
    while(processesCompleted < numberOfProcesses) {

        //Get Process to be Executed
        currentProcess = malloc(sizeof(process));
        dequeueProcessToExecute(currentProcess);

        //Check if there has been a Time Jump
        if(currentTime < currentProcess->timeArrived) {
            currentTime = currentProcess->timeArrived;
        }

        executedProcessID[processesCompleted] = currentProcess->processID;

        //Load the process onto memory if there is not unlimited memory
        if(strcasecmp(memoryAllocation,"u") != 0) {
            int loadTime;

            if(strcmp(memoryAllocation,"p") == 0) {
                loadTime = swappingX(currentProcess, memorySize, currentTime);
            } else if(strcmp(memoryAllocation,"v") == 0) {
                loadTime = virtualMemory(currentProcess, memorySize, currentTime);
            }else if(strcmp(memoryAllocation,"cm") == 0) {
                loadTime = mostFrequentlyUsed(currentProcess, memorySize, currentTime, memoryFrequency);
            }

            printRUNNING(currentTime, currentProcess->processID, currentProcess->jobTimeLeft, memorySize, loadTime);
            currentTime = currentTime + loadTime;

        }else {
            printRUNNINGWithUnlimited(currentTime, currentProcess->processID, currentProcess->jobTimeLeft);
        }

        //Execute the process in seconds based on the quantum
        currentProcess->jobTimeLeft = currentProcess->jobTimeLeft - quantum;
        if(currentProcess->jobTimeLeft <= 0) {
            currentTime = currentTime + (quantum - abs(currentProcess->jobTimeLeft));
        } else {
            currentTime = currentTime + quantum;
        }

        //See what processes have arrived since executing the current process and add them to the queue
        while((pointerBottomProcess != NULL) && pointerBottomProcess->thisProcess.timeArrived <= currentTime ) {
            tempPtr = malloc(sizeof(process));

            popFromListOfProcesses(tempPtr);
            enqueueProcessToExecute(tempPtr->timeArrived,tempPtr->processID,tempPtr->memorySizeRequired,tempPtr->jobTime,tempPtr->jobTimeLeft);
            numberOfProcesses++;
            free(tempPtr);
        }

        enqueueProcessID(currentProcess->processID);
        //Check if the Process Job is finished, If not Put it back into the queue
        if(currentProcess->jobTimeLeft <= 0) {

            processesCompleted++;

            if(strcasecmp(memoryAllocation,"u") != 0) {
                evictProcessFromMemory(memorySize, currentProcess->processID, currentTime);
            }

            printFINISHED(currentTime,currentProcess->processID, numberOfProcesses - processesCompleted);

            totalTurnaroundTime = totalTurnaroundTime + (currentTime - currentProcess->timeArrived);;
            int turnaround = currentTime - currentProcess->timeArrived;
            overhead[processesCompleted-1] = 1.0* turnaround/currentProcess->jobTime;


            //Check if we need to expand the Throughput array
            while(currentTime > (THROUGHPUT_INTERVAL_SIZE * throughputSize)){

                throughputSize++;
                throughput = realloc(throughput,sizeof(double) * throughputSize);
                throughput[throughputSize-1] = 0;

            }

            throughput[throughputSize-1]++;

        } else {
            enqueueProcessToExecute(currentProcess->timeArrived,currentProcess->processID,currentProcess->memorySizeRequired,currentProcess->jobTime,currentProcess->jobTimeLeft);
        }

        //If there is no process left to be executed wait until one comes in (Time Jump)
        if((pointerBottomQueue == NULL && pointerBottomProcess != NULL)) {
            tempPtr = malloc(sizeof(process));

            popFromListOfProcesses(tempPtr);
            enqueueProcessToExecute(tempPtr->timeArrived,tempPtr->processID,tempPtr->memorySizeRequired,tempPtr->jobTime,tempPtr->jobTimeLeft);
            numberOfProcesses++;
            free(tempPtr);
        }
        executedProcessID = realloc(executedProcessID, (sizeof(int) * processesCompleted) + 1);

        if(currentProcess->jobTimeLeft <= 0) {
            overhead = realloc(overhead, sizeof(double) * numberOfProcesses);
        }

        free(currentProcess);
    }

    //Print the statistics of the simulation
    printStatistics(throughput,throughputSize,totalTurnaroundTime,overhead,numberOfProcesses,currentTime);

    free(overhead);
    free(throughput);
    free(executedProcessID);
    free(memoryFrequency);
}


//Non-Preemptive Shortest Job First Scheduling Algorithm
void shortestJobNext(int memorySize, char memoryAllocation[]) {
    struct heap h;
    createHeap(&h);
    process * tempPtr;
    int currentTime = 0;
    int processesCompleted = 0;
    int totalTurnaroundTime = 0;
    int numberOfProcesses = 0;
    double * throughput = malloc(sizeof(double));
    double * overhead = malloc(sizeof(double));
    int throughputSize = 1;
    throughput[0] = 0;
    process * currentProcess;
    process * head;
    int * memoryFrequency= malloc(sizeof(int) * memorySize);
    for(int i = 0; i < memorySize; i++){
        memoryFrequency[i] = 0;
    }

    //Enqueue the processes to be executed with arrival time of 0
    while(pointerBottomProcess->thisProcess.timeArrived == 0) {
        head = malloc(sizeof(process));
        popFromListOfProcesses(head);
        heapPush(&h, head);
        numberOfProcesses++;

        if(pointerBottomProcess == NULL){
            break;
        }
    }

    //Execute Processes that have arrived
    while(processesCompleted < numberOfProcesses) {

        //Get Process to be Executed
        currentProcess = popHeapRoot(&h);

        //Check if there has been a Time Jump
        if(currentTime < currentProcess->timeArrived) {
            currentTime = currentProcess->timeArrived;
        }

        //Load the process onto memory if there is not unlimited memory
        if(strcmp(memoryAllocation,"u") != 0) {
            int loadTime;

            if(strcmp(memoryAllocation,"p") == 0) {
                loadTime = swappingX(currentProcess, memorySize, currentTime);

            } else if(strcmp(memoryAllocation,"v") == 0) {
                loadTime = virtualMemory(currentProcess, memorySize, currentTime);

            }else if(strcmp(memoryAllocation,"cm") == 0) {
                loadTime = mostFrequentlyUsed(currentProcess, memorySize, currentTime, memoryFrequency);
            }

            printRUNNING(currentTime, currentProcess->processID, currentProcess->jobTimeLeft, memorySize, loadTime);
            currentTime = currentTime + loadTime;

        }else {
            printRUNNINGWithUnlimited(currentTime, currentProcess->processID, currentProcess->jobTimeLeft);
        }

        //Update the current time of the simulation and set Job time left to 0 as SJF is Non Preemptive
        currentTime = currentTime + currentProcess->jobTime;
        currentProcess->jobTimeLeft = 0;

        //See what processes have arrived since executing the current process and add them to the queue
        while(((pointerBottomProcess != NULL) && pointerBottomProcess->thisProcess.timeArrived <= currentTime)) {

            tempPtr = malloc(sizeof(process));
            popFromListOfProcesses(tempPtr);
            heapPush(&h, tempPtr);

            numberOfProcesses++;
        }



        //Check if we need to expand the Throughput array
        while(currentTime > (THROUGHPUT_INTERVAL_SIZE * throughputSize)) {

            throughputSize++;
            throughput = realloc(throughput,sizeof(double) * throughputSize);
            throughput[throughputSize-1] = 0;

        }

        throughput[throughputSize-1]++;
        totalTurnaroundTime = totalTurnaroundTime + (currentTime - currentProcess->timeArrived);;
        int turnaroundTime = currentTime - currentProcess->timeArrived;
        overhead[processesCompleted] = 1.0* turnaroundTime/currentProcess->jobTime;
        enqueueProcessID(currentProcess->processID);


        processesCompleted++;

        //Evict Process from Memory Pages
        if(currentProcess->jobTimeLeft == 0) {
            if(strcasecmp(memoryAllocation,"u") != 0) {
                evictProcessFromMemory(memorySize, currentProcess->processID, currentTime);
            }
        }

        printFINISHED(currentTime,currentProcess->processID, numberOfProcesses - processesCompleted);

        //If there is no process left to be executed wait until one comes in (Time Jump)
        if((h.heapArray[0] == NULL && pointerBottomProcess != NULL)) {
            tempPtr = malloc(sizeof(process));
            popFromListOfProcesses(tempPtr);
            heapPush(&h, tempPtr);

            numberOfProcesses++;
        }

        overhead = realloc(overhead, sizeof(double) * numberOfProcesses);

        free(currentProcess);
    }

    //Print the statistics of the simulation
    printStatistics(throughput,throughputSize,totalTurnaroundTime,overhead,numberOfProcesses,currentTime);

    free(overhead);
    free(throughput);
    free(memoryFrequency);
    free(h.heapArray);
    freeHeap(&h);

}

//Print the Running of a process info when memory is unlimited
void printRUNNINGWithUnlimited(int currentTime, int processID, int remainingTime) {
    printf("%d, RUNNING, id=%d, remaining-time=%d\n", currentTime,processID,remainingTime);
}

//Print the Running of a process info when memory is limited
void printRUNNING(int currentTime, int processID, int remainingTime, int memorySize, int loadTime) {
    int pagesInMemory = (int) floor(1.0 * memorySize / MEMORY_PER_PAGE);

    int pagesFull = 0;

    for(int i = 0; i < pagesInMemory ; i++) {
        if(memory[i] != BLANK_MEMORY_PAGE) {
            pagesFull++;
        }
    }

    //Calculate the current memory Usage in percentage
    int memoryUsage = ceil((1.0*pagesFull/pagesInMemory * 100));

    printf("%d, RUNNING, id=%d, remaining-time=%d, load-time=%d, mem-usage=%d%%, mem-addresses=[", currentTime,processID,remainingTime,loadTime,memoryUsage);

    //Print all the pages which are being used by the current process
    int numberOfPagesBeingUsedByCurrentProcess = 0;
    for(int i = 0; i < pagesInMemory ; i++) {
        if(memory[i] == processID) {
            numberOfPagesBeingUsedByCurrentProcess++;
        }
    }

    int numberOfPagesPrinted = 0;
    for(int i = 0; i < pagesInMemory ; i++) {

        if(memory[i] == processID) {
            numberOfPagesPrinted++;
            printf("%d",i);
            if(numberOfPagesPrinted != numberOfPagesBeingUsedByCurrentProcess) {
                printf(",");
            }
        }
    }

    printf("]\n");
}

//Print the pages which were evicted from the memory
void printEVICTED(int currentTime, int memoryAddresses[], int numberOfMemoryAddresses) {
    printf("%d, EVICTED, mem-addresses=[",currentTime);

    //Sort the memory pages which have been evicted from smallest to largest
    qsort( memoryAddresses, numberOfMemoryAddresses, sizeof(int), compare );

    for(int i = 0; i < numberOfMemoryAddresses ; i++){
        printf("%d",memoryAddresses[i]);
        if(i != numberOfMemoryAddresses - 1){
            printf(",");
        }
    }

    printf("]\n");
}

//Print when a process has completely finished being executed
void printFINISHED(int currentTime, int processID, int processesRemaining) {
    printf("%d, FINISHED, id=%d, proc-remaining=%d\n",currentTime,processID,processesRemaining);
}

//Print the Statistics of the Simulation when it has finished
void printStatistics(double throughput[],int throughputSize,int totalTurnaroundTime, double overhead[], int numberOfProcesses,int simulationTime) {
    printf("Throughput %d, %d, %d\n",(int) ceil(findAverage(throughput,throughputSize)),(int) findMin(throughput,throughputSize),(int) findMax(throughput,throughputSize));
    printf("Turnaround time %d\n",(int) ceil(1.0 * totalTurnaroundTime/numberOfProcesses));
    printf("Time overhead %.2f %.2f\n",roundf((float) findMax(overhead,numberOfProcesses) * 100) / 100,roundf((float) findAverage(overhead,numberOfProcesses) * 100) / 100);
    printf("Makespan %d\n", simulationTime);
}

//Enqueue Process To list
void addToListOfProcesses(int timeArrived, int processId, int memorySizeRequired, int jobTime) {
    queueNode * newNode = malloc(sizeof(queueNode));
    assert(newNode);

    newNode->thisProcess.timeArrived = timeArrived;
    newNode->thisProcess.processID = processId;
    newNode->thisProcess.memorySizeRequired = memorySizeRequired;
    newNode->thisProcess.jobTime = jobTime;
    newNode->thisProcess.jobTimeLeft = jobTime;
    newNode->nextProcess = NULL;
    newNode->previousProcess = NULL;


    if(pointerTopProcess == NULL) {

        newNode->nextProcess = pointerTopProcess;
        pointerTopProcess = newNode;
        pointerBottomProcess = newNode;

        return;
    } else if((pointerTopProcess->thisProcess.timeArrived < timeArrived) || (pointerTopProcess != NULL && pointerTopProcess->thisProcess.timeArrived == timeArrived && pointerTopProcess->thisProcess.processID < processId)) {
        newNode->nextProcess = pointerTopProcess;
        pointerTopProcess = newNode;

        return;
    } else if(pointerTopProcess->nextProcess != NULL && pointerTopProcess->thisProcess.timeArrived < timeArrived ) {

        queueNode * nextNode = pointerTopProcess->nextProcess;
        newNode->nextProcess = nextNode;
        pointerTopProcess->nextProcess = newNode;
        return;
    } else {
        queueNode * left;
        queueNode * right = pointerTopProcess;

        while(right != NULL && right->thisProcess.timeArrived == timeArrived && right->thisProcess.processID > processId) {

            left = right;
            right = right->nextProcess;
        }
        left->nextProcess=newNode;

        newNode->nextProcess = right;
    }
}



//Dequeue Process To list
void popFromListOfProcesses(process * processToRun) {

    if(pointerBottomProcess == NULL) {
        return;
    }

    processToRun->processID = pointerBottomProcess->thisProcess.processID;
    processToRun->timeArrived = pointerBottomProcess->thisProcess.timeArrived;
    processToRun->jobTime = pointerBottomProcess->thisProcess.jobTime;
    processToRun->memorySizeRequired = pointerBottomProcess->thisProcess.memorySizeRequired;
    processToRun->jobTimeLeft = pointerBottomProcess->thisProcess.jobTimeLeft;


    queueNode *TempPointer = pointerBottomProcess;
    pointerBottomProcess = pointerBottomProcess->previousProcess;
    free(TempPointer);

    if(pointerBottomProcess == NULL) {
        pointerTopProcess = NULL;
    }
}

//Enqueue Process To queue
void enqueueProcessToExecute(int timeArrived, int processId, int memorySizeRequired, int jobTime, int jobTimeLeft) {
    queueNode * tempPointer = malloc(sizeof(queueNode));
    assert(tempPointer);

    tempPointer->thisProcess.timeArrived = timeArrived;
    tempPointer->thisProcess.processID = processId;
    tempPointer->thisProcess.memorySizeRequired = memorySizeRequired;
    tempPointer->thisProcess.jobTime = jobTime;
    tempPointer->thisProcess.jobTimeLeft = jobTimeLeft;
    tempPointer->nextProcess = NULL;

    if(pointerBottomQueue == NULL) {
        pointerBottomQueue = tempPointer;
    }
    else {
        pointerTopQueue->nextProcess = tempPointer;
    }
    pointerTopQueue = tempPointer;

}

//Dequeue Process
void dequeueProcessToExecute(process * processToRun) {

    if(pointerBottomQueue == NULL) {
        return;
    }

    processToRun->processID = pointerBottomQueue->thisProcess.processID;
    processToRun->timeArrived = pointerBottomQueue->thisProcess.timeArrived;
    processToRun->jobTime = pointerBottomQueue->thisProcess.jobTime;
    processToRun->memorySizeRequired = pointerBottomQueue->thisProcess.memorySizeRequired;
    processToRun->jobTimeLeft = pointerBottomQueue->thisProcess.jobTimeLeft;


    queueNode *TempPointer = pointerBottomQueue;
    pointerBottomQueue = pointerBottomQueue->nextProcess;
    free(TempPointer);

    if(pointerTopQueue == NULL) {
        pointerTopQueue = NULL;
    }
}

//To calculate average of an array
double findAverage(const double numbers[], int size) {

    double total = 0;
    for(int i = 0; i < size; i++) {
        total += numbers[i];
    }
    return total/size;
}

//Find the Maximum element of an array
double findMax(const double numbers[], int size) {

    double maximum = 0;
    for(int i = 0; i < size; i++) {
        if(numbers[i] > maximum) {
            maximum = numbers[i];
        }
    }
    return maximum;
}

//Find the minimum element of an array
double findMin(const double numbers[], int size) {

    double minimum = numbers[0];
    for(int i = 1; i < size; i++) {
        if(numbers[i] < minimum) {
            minimum = numbers[i];
        }
    }
    return minimum;
}

//Find the maximum Index of an array
int findMaxIndex(const int *numbers, int size) {


    printf("%d",size);
    int maximum = numbers[0];
    int maximumIndex = 0;
    for(int i = 1; i < size; i++) {
        if(numbers[i] > maximum) {
            maximum = numbers[i];
            maximumIndex = i;
        }
    }
    return maximumIndex;
}

//Enqueue to processID Linked List
void enqueueProcessID(int recentProcessID) {
     processID * tempPointer = malloc(sizeof(processID));
    assert(tempPointer);

    tempPointer->nextProcessID = NULL;

    if(pointerBottomID == NULL) {
        pointerBottomID = tempPointer;
    }
    else {
        pointerTopID->nextProcessID = tempPointer;
    }
    pointerTopID = tempPointer;

    pointerTopID->processID = recentProcessID;
}

//Dequeue from processID Linked List
int dequeueProcessID() {

    if(pointerBottomID == NULL) {
        return 0;
    }

    int leastPreviousProcessID = pointerBottomID->processID;

    processID * TempPointer = pointerBottomID;
    pointerBottomID = pointerBottomID->nextProcessID;
    free(TempPointer);

    if(pointerTopID == NULL) {
        pointerTopID = NULL;
    }
    return leastPreviousProcessID;
}

//Compare Helper Function to be used with the c qsort command
int compare(const void* a, const void* b) {
    int int_a = * ( (int*) a );
    int int_b = * ( (int*) b );

    //Compares int A to int B
    if ( int_a == int_b ) {
        return EQUAL;
    }
    else if ( int_a < int_b ) {
        return SMALLER;
    }
    else {
        return LARGER;
    }
}

