#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

void schedule() {
    struct node *previous = NULL;
    while (list_of_tasks) { /* reverse linked list so that it's ordered in FCFS fashion */
        struct node *current = list_of_tasks;
        list_of_tasks = list_of_tasks->next;
        current->next = previous;
        previous = current;
    }
    while (previous) {
        run(previous->task, previous->task->burst); /* execute each task */
        printf("Task %s finished.\n", previous->task->name);
        previous = previous->next;
    }
}
