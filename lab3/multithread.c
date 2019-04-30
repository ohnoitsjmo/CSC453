#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 
#include <time.h>

typedef struct {
  int *array;
  int used;
  int size;
} Array;

typedef struct {
    Array arg1;
    Array arg2;
} Args;

void initArray(Array *a, size_t initialSize) {
  a->array = (int *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(Array *a, int element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (int *)realloc(a->array, a->size * sizeof(int));
  }
  a->array[a->used++] = element;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

Array global_array;
Array sorted_array;

int compare_func(const void * a, const void * b) /* comparator for sorting algo */
{
    return ( *(int*)a - *(int*)b );
}

void* sortingThreads(void* param) { /* sorting algorithm */
    Array * array_of_ints = (Array *)param;
    qsort(array_of_ints->array, array_of_ints->used, sizeof(int), compare_func);
    return NULL;
}

void* mergeSorted(void* param) { /* merging algorithm */
    Args * args = (Args *)param;
    Array first_sublist = args->arg1;
    Array second_sublist = args->arg2;
    int * first_array = first_sublist.array;
    int * second_array = second_sublist.array;
    int first_counter = 0;
    int second_counter = 0;
    int total_counter = 0;
    while (first_counter < first_sublist.used && second_counter < second_sublist.used) {
        if (first_array[first_counter] < second_array[second_counter]) {
            sorted_array.array[total_counter] = first_array[first_counter];
            first_counter++;
        } else {
            sorted_array.array[total_counter] = second_array[second_counter];
            second_counter++;
        }
        total_counter++;
    }
    while (first_counter < first_sublist.used) {
        sorted_array.array[total_counter] = first_array[first_counter];
        first_counter++;
        total_counter++;
    }
    while (second_counter < second_sublist.used) {
        sorted_array.array[total_counter] = second_array[second_counter];
        second_counter++;
        total_counter++;
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    struct timespec start, finish;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *file = fopen(argv[1],"r");
    int n;
    initArray(&global_array, 3000);
    while (fscanf(file, " %d", &n) == 1) {
        insertArray(&global_array, n);
    }
    fclose(file);
    sorted_array.array = (int *)malloc(global_array.used * sizeof(int));
    sorted_array.size = global_array.used;
    sorted_array.used = global_array.used;

    pthread_t threads[3]; 
    /* create two sorting threads -- one to sort first half and the other to sort second half of array */

    Array first_sublist;
    Args args;
    initArray(&first_sublist, global_array.used/2);
    Array second_sublist;
    initArray(&second_sublist, global_array.used-global_array.used/2);
    memcpy(first_sublist.array, global_array.array, global_array.used/2 * sizeof(int));
    first_sublist.used = global_array.used/2;
    memcpy(second_sublist.array, global_array.array + global_array.used/2, (global_array.used-global_array.used/2) * sizeof(int));
    second_sublist.used = global_array.used-global_array.used/2;
    pthread_create(&threads[0], NULL, sortingThreads, (void *)&first_sublist); /* create first sorting thread */
    pthread_create(&threads[1], NULL, sortingThreads, (void *)&second_sublist); /* create second sorting thread */
    // sortingThreads((void *)&first_sublist); /* single thread implementation */
    // sortingThreads((void *)&second_sublist); /* single thread implementation */
    args.arg1 = first_sublist;
    args.arg2 = second_sublist;
    pthread_join(threads[0], NULL); /* wait for first thread to terminate */
    pthread_join(threads[1], NULL); /* wait for second thread to terminate */
    pthread_create(&threads[2], NULL, mergeSorted, (void *)&args); /* create third thread for merging */
    // mergeSorted((void *)&args); /* single thread implementation */
    FILE *output = fopen("output.txt", "ab+"); /* create/open output.txt file */
    pthread_join(threads[2], NULL); /* wait for third thread to terminate */
    for (int i = 0; i < sorted_array.used; i++) { /* iterate sorted array and output to output file */
        fprintf(output, "%d ", sorted_array.array[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Time elapsed: %f seconds\n", elapsed);
    exit(EXIT_SUCCESS); 
}
