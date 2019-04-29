#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

struct node * smallestTask(struct node * head) /* finds and returns the smallest task */
{ 
    struct node * smallest_task = NULL;
    int min = INT_MAX; 
  
    while (head != NULL) { 

        if (min > head->task->burst) {
            min = head->task->burst;
            smallest_task = head;
        }
  
        head = head->next; 
    }
     
    return smallest_task; 
} 

void schedule() {
    while (list_of_tasks) {
        struct node * smallest_task = smallestTask(list_of_tasks);
        run(smallest_task->task, smallest_task->task->burst); /* run each task */
        printf("Task %s finished.\n", smallest_task->task->name);
        delete(&list_of_tasks, smallest_task->task); /* remove the task once it's run */
    }
}
