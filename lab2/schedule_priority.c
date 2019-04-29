#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

struct node * highestPriority(struct node * head) /* finds and returns highest priority task */
{ 
    struct node * highest_priority = NULL;
    int max = 0; 
  
    while (head != NULL) { 

        if (max < head->task->priority) {
            max = head->task->priority;
            highest_priority = head;
        }
  
        head = head->next; 
    }
     
    return highest_priority; 
} 

void schedule() {
    while (list_of_tasks) {
        struct node * highest_priority = highestPriority(list_of_tasks);
        run(highest_priority->task, highest_priority->task->burst); /* run each task */
        printf("Task %s finished.\n", highest_priority->task->name);
        delete(&list_of_tasks, highest_priority->task); /* remove the task once it's run */
    }
}
