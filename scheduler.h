/*
 *              scheduler.h
 * Computer Systems (COMP30023) - Project 1
 * Adam Lawrence || arlawrence || 992684
 * arlawrence@student.unimelb.edu.au
 *
 */

#ifndef CODE_SCHEDULER_H
#define CODE_SCHEDULER_H

/* Define Constants */

#define NULL_BYTE 1
#define MEMORY_PER_PAGE 4
#define VIRTUAL_MEMORY_MIN_PAGES_NEEDED 4
#define BLANK_MEMORY_PAGE -1
#define DEFUALT_QUANTUM 10
#define STRING_SIZE 1024
#define OPTION_SIZE 2
#define PENALTY_PER_PAGE_LOADED 2

/* Struct Typedefs */

//Struct for an individual Process
typedef struct process {
    int timeArrived;
    int processID;
    int memorySizeRequired;
    int jobTime;
    int jobTimeLeft;
} process;

typedef struct queueNode{
    struct process thisProcess;
    struct queueNode * nextProcess;
    struct queueNode * previousProcess;

} queueNode;

/* Function Prototypes */
int mostFrequentlyUsed(process * currentProcess, int memorySize, int currentTime, int * memoryFrequency);
void createMemory(int memorySize);
int swappingX(process * currentProcess, int memorySize, int currentTime);
int virtualMemory(process * currentProcess, int memorySize,int currentTime);
void evictProcessFromMemory(int memorySize, int processID, int currentTime);

#endif //CODE_SCHEDULER_H
