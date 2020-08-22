//
// Created by Adam on 16/05/2020.
//

#ifndef CODE_SCHEDULINGALGORITHMS_H
#define CODE_SCHEDULINGALGORITHMS_H

/* Define Constants */
#define NULL_BYTE 1
#define THROUGHPUT_INTERVAL_SIZE 60
#define EQUAL 0
#define SMALLER -1
#define LARGER 1

/* Struct Typedefs */
typedef struct processID{
    int processID;
    struct processID * nextProcessID;
} processID;

/* Function Prototypes */
void firstComeFirstServe(int memorySize, char memoryAllocation[]);
void roundRobin(int memorySize,int quantum, char memoryAllocation[]);
void shortestJobNext(int memorySize, char memoryAllocation[]);

void printRUNNINGWithUnlimited(int currentTime, int processID, int remainingTime);
void printRUNNING(int currentTime, int processID, int remainingTime, int memorySize, int loadTime);
void printEVICTED(int currentTime, int memoryAddresses[], int numberOfMemoryAddresses);
void printFINISHED(int currentTime, int processID, int processesRemaining);
void printStatistics(double throughput[],int throughputSize,int totalTurnaroundTime, double overhead[], int numberOfProcesses,int simulationTime);

void addToListOfProcesses(int timeArrived, int processId, int memorySizeRequired, int jobTime);
void popFromListOfProcesses(process * processToRun);
void enqueueProcessToExecute(int timeArrived, int processId, int memorySizeRequired, int jobTime, int jobTimeLeft);
void dequeueProcessToExecute(process * processToRun);
void enqueueProcessID(int recentProcessID);
int dequeueProcessID();

int findMaxIndex(const int *numbers, int size);
double findMin(const double numbers[], int size);
double findMax(const double numbers[], int size);
double findAverage(const double numbers[], int size);
int compare( const void * a, const void* b);

#endif //CODE_SCHEDULINGALGORITHMS_H
