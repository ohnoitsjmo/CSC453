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

void round_robin(struct node * list_of_tasks) {
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

void schedule() {
    int counter = 0;
    struct node * temp = list_of_tasks;
    while (temp) { /* get length of linked list */
        counter++;
        temp = temp->next;
    }
    int array_size = counter;
    struct node * ordered_tasks[array_size];
    counter = 0;
    while (list_of_tasks) {
        struct node * highest_priority = highestPriority(list_of_tasks);
        ordered_tasks[counter] = highest_priority;
        delete(&list_of_tasks, ordered_tasks[counter]->task);    
        counter++;
    }
    int duplicate_counts[101] = {0};
    for (int i = 0; i < 101; i++) {
        duplicate_counts[i] = 1;
    }
    for (int i = 0; i < array_size-1; i++) {
        int priority = ordered_tasks[i]->task->priority;
        if (priority == ordered_tasks[i+1]->task->priority)
            duplicate_counts[priority]++;
    } 
    for (int i = 0; i < array_size; i++) {
        struct node * curr_task = ordered_tasks[i];
        if (duplicate_counts[curr_task->task->priority] != 1) {
            struct node * sub = NULL;  /* sub-linked list */
            insert(&sub, curr_task->task);
            for (int j = 1; j < duplicate_counts[curr_task->task->priority]; j++) {    /* construct sub-linked list of duplicates */
                insert(&sub, ordered_tasks[i+j]->task);
            }
            round_robin(sub);  /* if duplicates, run round robin algorithm */
            i += duplicate_counts[curr_task->task->priority]-1;
        } else {
            run(ordered_tasks[i]->task, ordered_tasks[i]->task->burst); /* run entire task */
            printf("Task %s finished.\n", ordered_tasks[i]->task->name);
        }
    }
}