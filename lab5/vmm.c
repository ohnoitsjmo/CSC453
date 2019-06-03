#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
 
#define PAGE_SIZE 256
#define MEMORY_SIZE 65536
#define BUF_SIZE 10
  
char main_memory[MEMORY_SIZE];
int page_table[PAGE_SIZE];

void init_page_table() { /* initialize page table entries with -1 */
    for (int i = 0; i < PAGE_SIZE; i++)
        page_table[i] = -1;  
}

void translate(char *buf, int *next_ptr, char *backing, FILE *output) {
    int virtual_address = atoi(buf); /* cast virtual address string to int */
    int offset = virtual_address & 255;
    int page_num = (virtual_address >> 8) & 255;
    int p_entry = page_table[page_num];
    if (p_entry == -1) { /* if page_table entry is -1 then page fault */                
        p_entry = *next_ptr;
        void * page_frame = main_memory + (p_entry * PAGE_SIZE);
        const void * backing_page = backing + (page_num * PAGE_SIZE);
        memcpy(page_frame, backing_page, PAGE_SIZE); /* memcpy BACKING_STORE.bin page to physical memory */
        page_table[page_num] = p_entry; /* set table entry to physical address if page fault */
        (*next_ptr)++;  
    }
    int physical_address = (p_entry << 8) | offset;
    char value = main_memory[p_entry * PAGE_SIZE + offset];

    fprintf(output,"Virtual address: %d Physical address: %d Value: %d\n", virtual_address, physical_address, value); /* output to output.txt */
}
 
int main(int argc, const char *argv[])
{     
    char buf[BUF_SIZE];
    char *backing_store = "BACKING_STORE.bin"; 
    const char *input_file = argv[1];
    const char *output_file = "output.txt";
    int file_descriptor = open(backing_store, O_RDONLY); /* open BACKING_STORE.bin with read only permissions */
    init_page_table();
    char *backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, file_descriptor, 0); /* backing pointer to read in page from BACKING_STORE.bin */
    int next_ptr = 0;
    FILE *input = fopen(input_file, "r");
    FILE *output = fopen(output_file, "w");

    while (fgets(buf, BUF_SIZE, input) != NULL)
        translate(buf, &next_ptr, backing, output);
     
    exit(EXIT_SUCCESS);
}