#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

void circ_delete(struct node **head, Task *task) { 
    struct node *temp;
    struct node *prev;

    temp = *head;
    /* if only one element left, delete */
    if (strcmp((*head)->task->name, (*head)->next->task->name) == 0) {
        *head = NULL;
        return;
    }
    /* interior or last element in the list */
    prev = *head;
    temp = temp->next;
    while (strcmp(task->name,temp->task->name) != 0) {
        prev = temp;
        temp = temp->next;
    }

    prev->next = temp->next;
} 

void schedule() {
    struct node * temp = list_of_tasks;
    while (list_of_tasks->next) {
        list_of_tasks = list_of_tasks->next;
    }
    list_of_tasks->next = temp; /* set last node to point to first node to create circular linked list */
    list_of_tasks = list_of_tasks->next;
    int should_run = 1;
    while (should_run) {
        int curr_burst = list_of_tasks->task->burst;
        int remaining_burst = (curr_burst - QUANTUM);
        if (remaining_burst <= 0) {
            run(list_of_tasks->task, curr_burst);
            printf("Task %s finished.\n", list_of_tasks->task->name);
            circ_delete(&list_of_tasks, list_of_tasks->task);
            if (!list_of_tasks)
                break;
        } else {
            run(list_of_tasks->task, QUANTUM);
            list_of_tasks->task->burst = (curr_burst - QUANTUM);
        }
        list_of_tasks = list_of_tasks->next;
    }
}
