#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_LINE 80

int max;

typedef struct {
    char processName[3];
    char processStrategy[2];
    int processBytes, processStart, processEnd; 
} Process; 

typedef struct {
  Process *array;
  int used;
  int size;
} ProcessArray; 

void initArray(ProcessArray *a, size_t initialSize) { /* initializes array */
  a->array = (Process *)malloc(initialSize * sizeof(Process));
  a->used = 0;
  a->size = initialSize;
}

int compare_func(const void * a, const void * b) /* comparator for sorting algo */
{
    return ( ((Process*)a)->processStart - ((Process*)b)->processStart );
}

void* sortArray(void* param) { /* sorting algorithm */
    ProcessArray * array_of_processes = (ProcessArray *)param;
    qsort(array_of_processes->array, array_of_processes->used, sizeof(Process), compare_func);
    return NULL;
}

void insertArray(ProcessArray *a, Process element) { /* inserts process into process array */
    if (a->used == a->size) {
        a->size *= 2;
        a->array = (Process *)realloc(a->array, a->size * sizeof(Process));
    }
    a->array[a->used++] = element;
    sortArray(a);
}

void deleteArray(ProcessArray *a, char* processName) { /* removes specified process from array */
    for (int i = 0; i < a->used; i++) {
        if (!strcmp(a->array[i].processName, processName)) {
            for (int j = i; j < a->used-1; j++) {
                a->array[j] = a->array[j+1];
            }
            (a->used)--;
            return;
        }
    }
}

void freeArray(ProcessArray *a) { /* frees memory to prevent memory leaks */
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

void firstFit(Process *process, ProcessArray *array_of_processes) { /* first fit algorithm */
    for (int i = 0; i < array_of_processes->used - 1; i++) { /* looks for first open hole that process fits in */
        Process curr_process = array_of_processes->array[i];
        if ((i==0) && (curr_process.processStart != 0) && (process->processBytes <= curr_process.processStart)) {
            process->processStart = 0;
            process->processEnd = process->processBytes-1;
            insertArray(array_of_processes, *process);
            return;
        } if (curr_process.processEnd != (array_of_processes->array[i+1].processStart-1)) {
            if ((array_of_processes->array[i+1].processStart - curr_process.processEnd) >= process->processBytes) {
                process->processStart = curr_process.processEnd+1;
                process->processEnd = process->processStart+(process->processBytes-1);
                insertArray(array_of_processes, *process);
                return;
            } 
        }
    }
    if ((max - array_of_processes->array[array_of_processes->used-1].processEnd) > process->processBytes) {
        process->processStart = array_of_processes->array[array_of_processes->used-1].processEnd+1;
        process->processEnd = process->processStart+(process->processBytes-1);
        insertArray(array_of_processes, *process);
    } else {
        printf("Requesting too much memory\n");
    }
}

void bestFit(Process *process, ProcessArray *array_of_processes) { /* best fit algorithm */
    int minimum = INT_MAX;
    int temp;
    for (int i = 0; i < array_of_processes->used - 1; i++) { /* looks for smallest open hole that process fits in */
        Process curr_process = array_of_processes->array[i];
        if ((i==0) && (curr_process.processStart < minimum) && (curr_process.processStart >= process->processBytes)) {
            minimum = curr_process.processStart;
            process->processStart = 0;
            process->processEnd = process->processBytes-1;
        }
        temp = array_of_processes->array[i+1].processStart - curr_process.processEnd;
        if (temp < minimum && temp > process->processBytes) {
            minimum = temp;
            process->processStart = curr_process.processEnd+1;
            process->processEnd = process->processStart+(process->processBytes-1);
        }
    }
    temp = max-(array_of_processes->array[array_of_processes->used-1].processEnd);
    if (temp < minimum && temp > process->processBytes) {
        minimum = temp;
        process->processStart = array_of_processes->array[array_of_processes->used-1].processEnd+1;
        process->processEnd = process->processStart+(process->processBytes-1);
    } if (minimum == INT_MAX) {
        printf("Requesting too much memory\n");
    } else {
        insertArray(array_of_processes, *process);
    }
}

void worstFit(Process *process, ProcessArray *array_of_processes) { /* worst fit algorithm */
    int maximum = INT_MIN;
    int temp;
    for (int i = 0; i < array_of_processes->used - 1; i++) { /* looks for largest open hole that process fits in */
        Process curr_process = array_of_processes->array[i];
        if ((i==0) && (curr_process.processStart > maximum) && (curr_process.processStart >= process->processBytes)) {
            maximum = curr_process.processStart;
            process->processStart = 0;
            process->processEnd = process->processBytes-1;
        }
        temp = array_of_processes->array[i+1].processStart - curr_process.processEnd;
        if (temp > maximum && temp > process->processBytes) {
            maximum = temp;
            process->processStart = curr_process.processEnd+1;
            process->processEnd = process->processStart+(process->processBytes-1);
        }
    }
    temp = max-(array_of_processes->array[array_of_processes->used-1].processEnd);
    if (temp > maximum && temp > process->processBytes) {
        maximum = temp;
        process->processStart = array_of_processes->array[array_of_processes->used-1].processEnd+1;
        process->processEnd = process->processStart+(process->processBytes-1);
    } if (maximum == INT_MIN) {
        printf("Requesting too much memory\n");
    } else {
        insertArray(array_of_processes, *process);
    }
}

void compaction(ProcessArray *array_of_processes) { /* compacts holes of memory */
    for (int i=0; i < array_of_processes->used; i++) {
        if (i==0) {
            array_of_processes->array[i].processStart = 0;
            array_of_processes->array[i].processEnd = array_of_processes->array[i].processBytes-1;
        } else {
            array_of_processes->array[i].processStart = array_of_processes->array[i-1].processEnd+1;
            array_of_processes->array[i].processEnd = array_of_processes->array[i].processStart+(array_of_processes->array[i].processBytes-1);
        }
    }
}

int main(int argc, char *argv[])
{
    char buf[MAX_LINE];
    char *p, *request;
    Process *process;
    ProcessArray array_of_processes;
    max = strtol(argv[1], &p, 10); /* stores initial memory amount */
    
    initArray(&array_of_processes, 1);
    printf("allocator>");
    while (1) {                                                            
        fgets(buf, MAX_LINE, stdin);  
        request = strtok(buf, " ");
        if (!strcmp("X\n", request)) {
            break;
        } else if (!strcmp("RQ", request)) { /* request conditional */
            char * processName = strtok(NULL, " ");
            int processBytes = atoi(strtok(NULL, " "));
            char * processStrategy = strtok(NULL, " \n");
            if (processBytes > max) {
                printf("Requesting too much memory\n");
            } else {
                process = (Process*)malloc(sizeof(Process));
                strcpy(process->processName, processName);
                process->processBytes = processBytes;
                strcpy(process->processStrategy, processStrategy);
                if (array_of_processes.used == 0) {
                    process->processStart = 0;
                    process->processEnd = process->processBytes - 1;
                    insertArray(&array_of_processes, *process);
                } else if (!strcmp("F", process->processStrategy)) {
                    firstFit(process, &array_of_processes);
                } else if (!strcmp("B", process->processStrategy)) {
                    bestFit(process, &array_of_processes);
                } else if (!strcmp("W", process->processStrategy)) {
                    worstFit(process, &array_of_processes);
                }
            }
        } else if (!strcmp("STAT\n", request)) { /* prints processes and holes in memory */
            if (array_of_processes.used == 0) {
                printf("Addresses [0:%d] Unused\n", max-1);
                printf("allocator>");
                continue;
            } else if (array_of_processes.used == 1 && (array_of_processes.array[0].processStart != 0)) {
                printf("Addresses [0:%d] Unused\n", array_of_processes.array[0].processStart-1);
            }
            for (int i = 0; i < array_of_processes.used - 1; i++) { 
                Process curr_process = array_of_processes.array[i];

                if (i == 0 && (curr_process.processStart != 0)) {
                    printf("Addresses [0:%d] Unused\n", curr_process.processStart-1);
                }   
                printf("Addresses [%d:%d] Process %s\n", curr_process.processStart, curr_process.processEnd, curr_process.processName);
                if (curr_process.processEnd != (array_of_processes.array[i+1].processStart - 1)) {
                    printf("Addresses [%d:%d] Unused\n", curr_process.processEnd+1, array_of_processes.array[i+1].processStart-1);
                }
            }
            printf("Addresses [%d:%d] Process %s\n", array_of_processes.array[array_of_processes.used-1].processStart, array_of_processes.array[array_of_processes.used-1].processEnd, array_of_processes.array[array_of_processes.used-1].processName);
            if (array_of_processes.array[array_of_processes.used-1].processEnd != max-1) {
                printf("Addresses [%d:%d] Unused\n", array_of_processes.array[array_of_processes.used-1].processEnd+1, max-1);
            }
        }   else if (!strcmp("RL", request)) { /* release process conditional */
            char * processName = strtok(NULL, " \n");
            deleteArray(&array_of_processes, processName);
        }   else if (!strcmp("C\n", request)) { /* compacts all holes into a single hole */
            compaction(&array_of_processes);
        }
        printf("allocator>");
    }
    freeArray(&array_of_processes);
}