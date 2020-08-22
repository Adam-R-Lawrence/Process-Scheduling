/*
 *              scheduler.c
 * Computer Systems (COMP30023) - Project 2
 * Adam Lawrence || arlawrence || 992684
 * arlawrence@student.unimelb.edu.au
 *
 * Scheduling simulation
 *
 */

//Libraries used
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "scheduler.h"
#include "schedulingAlgorithms.h"
#include <getopt.h>

extern queueNode * pointerTopQueue;
extern queueNode * pointerBottomQueue;
extern queueNode * pointerTopProcess;
extern queueNode * pointerBottomProcess;
extern processID * pointerTopID;
extern processID * pointerBottomID;
int * memory;
int * bitNumbers;

int main(int argc,char *argv[]) {

    char filename[STRING_SIZE + NULL_BYTE];
    char schedulingAlgorithm[OPTION_SIZE+NULL_BYTE];
    char memoryAllocation[OPTION_SIZE+NULL_BYTE];
    int memorySize = 0;
    int quantum = DEFUALT_QUANTUM;

    //Get all the argument values from the command line
    int option;
    while ((option = getopt(argc, argv, "f:a:m:s:q:")) != -1) {
        switch (option) {
            case 'f':
                strcpy(filename,optarg);
                break;
            case 'a':
                strcpy(schedulingAlgorithm,optarg);
                break;
            case 'm':
                strcpy(memoryAllocation,optarg);
                break;
            case 's':
                memorySize = (int) strtol(optarg,NULL,10);
                break;
            case 'q':
                quantum = (int) strtol(optarg,NULL,10);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    FILE * filePointer;
    int numberOfProcesses = 0;
    char str[STRING_SIZE];
    int timeArrived;
    int processID;
    int memorySizeRequired;
    int jobTime;

    //Open the file
    filePointer = fopen(filename, "r");
    if (filePointer == NULL){
        exit(EXIT_FAILURE);
    }

    //Put all the processes into a linked list
    while (fgets(str, STRING_SIZE, filePointer) != NULL) {

        numberOfProcesses++;
        timeArrived = (int) strtol(strtok(str, " "), NULL, 10);
        processID = (int) strtol(strtok(NULL, " "), NULL, 10);
        memorySizeRequired = (int) strtol(strtok(NULL, " "), NULL, 10);
        jobTime = (int) strtol(strtok(NULL, " "), NULL, 10);
        addToListOfProcesses(timeArrived,processID,memorySizeRequired,jobTime);
    }

    queueNode *temp = pointerTopProcess;
    while(temp->nextProcess != NULL){

        temp->nextProcess->previousProcess = temp;

        temp = temp->nextProcess;
        if(temp->nextProcess == NULL){
            pointerBottomProcess = temp;
        }
    }


    //If not unlimited memory, than create the memory pages for the Scheduling Algorithms.
    if(strcmp(schedulingAlgorithm,"u") != 0) {
        createMemory(memorySize);
    }

    if(strcmp(memoryAllocation,"cm") == 0) {
        bitNumbers = malloc(sizeof(int) * (int) floor(1.0 * memorySize / MEMORY_PER_PAGE));
        for (int i = 0; i < (int) floor(1.0 * memorySize / MEMORY_PER_PAGE); i++) {
            bitNumbers[i] = 0;
        }
    }

    //Start the simulation based on the scheduling algorithm
    if(strcmp(schedulingAlgorithm,"ff") == 0) {
        firstComeFirstServe(memorySize, memoryAllocation);
    } else if(strcmp(schedulingAlgorithm,"rr") == 0){
        roundRobin(memorySize,quantum, memoryAllocation);
    } else if(strcmp(schedulingAlgorithm,"cs") == 0){
        shortestJobNext(memorySize, memoryAllocation);
    }

    fclose(filePointer);
}

/*
 * Create memory pages for the processes
 * - If a memory page is empty, set it to BLANK_MEMORY_PAGE
 * - If a memory page is occupied by a process, set its value to equal the ID of that occupying process
 */
void createMemory(int memorySize){
    int pagesOfMemory = (int) floor(1.0 * memorySize/MEMORY_PER_PAGE);
    memory = malloc(sizeof(int) * pagesOfMemory);

    for(int i = 0; i < pagesOfMemory; i++){
        memory[i] = BLANK_MEMORY_PAGE;
    }
}

/*
 * Implementation of the SwappingX Memory Allocation Algorithm
 */
int swappingX(process * currentProcess, int memorySize, int currentTime) {
    int pagesInMemory = (int) floor(1.0 * memorySize / MEMORY_PER_PAGE);
    int pagesOfMemoryNeeded = (int) floor(1.0 * currentProcess->memorySizeRequired / MEMORY_PER_PAGE);
    int pagesOfMemoryStillNeeded = pagesOfMemoryNeeded;
    int * listOfPagesEvicted = malloc(sizeof(int));
    int numberOfPagesEvicted = 0;
    int timeRequirement = 0;
    int numberOfFreePages = 0;
    int numberOfPagesAlreadyLoaded = 0;

    /*
     * Calculate how many pages of memory are free and
     * calculate how many pages of memory of the current
     * Process are already loaded onto memory
     *
     */
    for (int i = 0; i < pagesInMemory; i++) {
        if (memory[i] == BLANK_MEMORY_PAGE) {
            numberOfFreePages++;
        }

        if(currentProcess->processID == memory[i]){
            numberOfPagesAlreadyLoaded++;
        }
    }

    //Check if all pages are already loaded
    if(numberOfPagesAlreadyLoaded != pagesOfMemoryNeeded) {

        //Remove Processes from memory until we have enough memory for the current process
        while (numberOfFreePages < pagesOfMemoryNeeded) {
            int leastRecentlyExecutedProcess = dequeueProcessID();

            for (int i = 0; i < pagesInMemory; i++) {
                if (leastRecentlyExecutedProcess == memory[i]) {
                    memory[i] = BLANK_MEMORY_PAGE;
                    numberOfPagesEvicted++;
                    numberOfFreePages++;
                    listOfPagesEvicted = realloc(listOfPagesEvicted, sizeof(int) * numberOfPagesEvicted);
                    listOfPagesEvicted[numberOfPagesEvicted - 1] = i;
                }
            }
        }

        //Load Process on free memory pages
        for (int i = 0; i < pagesInMemory; i++) {
            if (memory[i] == BLANK_MEMORY_PAGE) {
                memory[i] = currentProcess->processID;
                pagesOfMemoryStillNeeded--;

                if (pagesOfMemoryStillNeeded == 0) {
                    break;
                }
            }
        }

        //Each page loaded onto memory takes 2 seconds
        timeRequirement = PENALTY_PER_PAGE_LOADED * (pagesOfMemoryNeeded - numberOfPagesAlreadyLoaded);
    }

    //Print the Pages evicted from memory
    if (numberOfPagesEvicted != 0) {
        printEVICTED(currentTime, listOfPagesEvicted, numberOfPagesEvicted);
    }

    free(listOfPagesEvicted);

    return timeRequirement;
}

/*
 * Implementation of the Virtual Memory Allocation Algorithm
 */
int virtualMemory(process * currentProcess, int memorySize, int currentTime) {

    int pagesInMemory = (int) floor(1.0 * memorySize / MEMORY_PER_PAGE);
    int *pagesEvicted = malloc(sizeof(int));
    int numberOfPagesEvicted = 0;
    int pagesOfMemoryNeeded = (int) floor(1.0 * currentProcess->memorySizeRequired / MEMORY_PER_PAGE);
    int pagesOfMemoryStillToLoad = pagesOfMemoryNeeded;
    int numberOfPagesNotLoadedOntoMemory = pagesOfMemoryNeeded;
    int numberOfFreePages = 0;
    int numberOfPagesLoadedOntoMemory = 0;
    int numberOfPagesAlreadyLoaded = 0;

    /*
    * Calculate how many pages of memory are free and
    * calculate how many pages of memory of the current
    * Process are already loaded onto memory
    *
    */
    for (int i = 0; i < pagesInMemory; i++) {

        if (memory[i] == BLANK_MEMORY_PAGE) {

            numberOfFreePages++;
        }

        if(currentProcess->processID == memory[i]){
            pagesOfMemoryStillToLoad--;
            numberOfPagesNotLoadedOntoMemory--;
            numberOfPagesAlreadyLoaded++;

        }
    }

    //Remove pages until we have enough
    while(((numberOfFreePages + numberOfPagesAlreadyLoaded) < VIRTUAL_MEMORY_MIN_PAGES_NEEDED )) {
        int leastExecutedProcess;

        if(numberOfFreePages != pagesOfMemoryStillToLoad) {
            int test = 0;

            leastExecutedProcess = pointerBottomID->processID;
            if (leastExecutedProcess == currentProcess->processID) {
                dequeueProcessID();
                enqueueProcessID(currentProcess->processID);
            }

            for (int i = 0; i < pagesInMemory; i++) {

                if (leastExecutedProcess == memory[i] && currentProcess->processID != memory[i]) {


                    memory[i] = BLANK_MEMORY_PAGE;
                    numberOfPagesEvicted++;
                    pagesEvicted = realloc(pagesEvicted, sizeof(int) * numberOfPagesEvicted);
                    pagesEvicted[numberOfPagesEvicted - 1] = i;
                    numberOfFreePages++;


                    if ((numberOfFreePages + numberOfPagesAlreadyLoaded) == VIRTUAL_MEMORY_MIN_PAGES_NEEDED ||
                        numberOfFreePages == pagesOfMemoryStillToLoad) {

                        break;
                    }
                }
            }

            for (int i = 0; i < pagesInMemory; i++) {
                if (leastExecutedProcess == memory[i]) {
                    test++;
                }
            }

            if (test == 0) {
                dequeueProcessID();
            }

        }

        if ((numberOfFreePages + numberOfPagesAlreadyLoaded) == VIRTUAL_MEMORY_MIN_PAGES_NEEDED || numberOfFreePages == pagesOfMemoryStillToLoad) {
            break;
        }
    }

    //Load the pages Needed
    if(numberOfPagesAlreadyLoaded != pagesOfMemoryNeeded) {
        for (int i = 0; i < pagesInMemory; i++) {

            if (memory[i] == BLANK_MEMORY_PAGE) {
                memory[i] = currentProcess->processID;


                pagesOfMemoryStillToLoad--;
                numberOfPagesLoadedOntoMemory++;
                if (pagesOfMemoryStillToLoad == 0) {
                    break;
                }
            }
        }
    }

    //Increase how much job time is left for the current process, + 1 for each page not loaded onto memory
    currentProcess->jobTimeLeft =  currentProcess->jobTimeLeft +  (numberOfPagesNotLoadedOntoMemory - numberOfPagesLoadedOntoMemory);

    if (numberOfPagesEvicted != 0) {
        printEVICTED(currentTime, pagesEvicted, numberOfPagesEvicted);
    }

    free(pagesEvicted);

    //Return the time requirement for loading, 2 * Number Of Pages Loaded Onto Memory
    return PENALTY_PER_PAGE_LOADED * numberOfPagesLoadedOntoMemory;;
}

/*
 * Implementation of the Most Frequently Used Memory Management for my Custom Memory Management
 */
int mostFrequentlyUsed(process * currentProcess, int memorySize, int currentTime, int * memoryFrequency) {

    int pagesInMemory = (int) floor(1.0 * memorySize / MEMORY_PER_PAGE);
    int *pagesEvicted = malloc(sizeof(int));
    int numberOfPagesEvicted = 0;
    int numberOfPagesNeeded = (int) floor(1.0 * currentProcess->memorySizeRequired / MEMORY_PER_PAGE);
    int pagesOfMemoryStillNeeded = numberOfPagesNeeded;
    int numberOfPagesNotLoaded = numberOfPagesNeeded;
    int numberOfFreePages = 0;
    int numberOfPagesLoadedOntoMemory = 0;
    int numberOfPagesAlreadyLoaded = 0;

    /*
    * Calculate how many pages of memory are free and
    * calculate how many pages of memory of the current
    * Process are already loaded onto memory
    *
    */
    for (int i = 0; i < pagesInMemory; i++) {

        if (memory[i] == BLANK_MEMORY_PAGE) {
            numberOfFreePages++;
        }

        if(currentProcess->processID == memory[i]){
            pagesOfMemoryStillNeeded--;
            numberOfPagesNotLoaded--;
            numberOfPagesAlreadyLoaded++;

            //This page of memory is being referenced again, thus increment its counter by 1
            memoryFrequency[i]++;
        }
    }

    //Remove pages until we have enough
    while (((numberOfFreePages + numberOfPagesAlreadyLoaded) < VIRTUAL_MEMORY_MIN_PAGES_NEEDED )) {
        int mostFrequentlyUsedIndex;

        if(numberOfFreePages != pagesOfMemoryStillNeeded) {

            //Remove the page which has the highest counter
            mostFrequentlyUsedIndex = findMaxIndex(memoryFrequency, (int) floor(1.0 * memorySize / MEMORY_PER_PAGE));
            memory[mostFrequentlyUsedIndex] = BLANK_MEMORY_PAGE;
            numberOfPagesEvicted++;
            pagesEvicted = realloc(pagesEvicted, sizeof(int) * numberOfPagesEvicted);
            pagesEvicted[numberOfPagesEvicted - 1] = mostFrequentlyUsedIndex;
            numberOfFreePages++;

            //This page of memory is being referenced again, thus increment its counter by 1
            memoryFrequency[mostFrequentlyUsedIndex]++;

            if ((numberOfFreePages + numberOfPagesAlreadyLoaded) == VIRTUAL_MEMORY_MIN_PAGES_NEEDED ||
                numberOfFreePages == pagesOfMemoryStillNeeded) {

                break;
            }
        }

        if ((numberOfFreePages + numberOfPagesAlreadyLoaded) == VIRTUAL_MEMORY_MIN_PAGES_NEEDED || numberOfFreePages == pagesOfMemoryStillNeeded) {
            break;
        }
    }

    //Load the pages Needed
    if(numberOfPagesAlreadyLoaded != numberOfPagesNeeded) {
        for (int i = 0; i < pagesInMemory; i++) {

            if (memory[i] == BLANK_MEMORY_PAGE) {
                memory[i] = currentProcess->processID;


                pagesOfMemoryStillNeeded--;
                numberOfPagesLoadedOntoMemory++;
                if (pagesOfMemoryStillNeeded == 0) {
                    break;
                }
            }
        }
    }

    //Increase how much job time is left for the current process, + 1 for each page not loaded onto memory
    currentProcess->jobTimeLeft =  currentProcess->jobTimeLeft +  (numberOfPagesNotLoaded - numberOfPagesLoadedOntoMemory);

    if (numberOfPagesEvicted != 0) {
        printEVICTED(currentTime, pagesEvicted, numberOfPagesEvicted);
    }

    free(pagesEvicted);

    //Return the time requirement for loading, 2 * Number Of Pages Loaded Onto Memory
    return PENALTY_PER_PAGE_LOADED * numberOfPagesLoadedOntoMemory;;
}

//Remove all pages of memory associated with a Finished Process
void evictProcessFromMemory(int memorySize, int processID, int currentTime){
    int pagesInMemory = (int) floor(1.0 * memorySize / MEMORY_PER_PAGE);
    int numberOfPagesEvicted = 0;
    int * pagesEvicted = malloc(sizeof(int));

    for(int i = 0; i< pagesInMemory; i++){
        if(memory[i] == processID){
            memory[i] = BLANK_MEMORY_PAGE;
            numberOfPagesEvicted++;
            pagesEvicted = realloc(pagesEvicted,sizeof(int) * numberOfPagesEvicted);
            pagesEvicted[numberOfPagesEvicted - 1] = i;
        }
    }

    printEVICTED(currentTime,pagesEvicted,numberOfPagesEvicted);
    free(pagesEvicted);
}
