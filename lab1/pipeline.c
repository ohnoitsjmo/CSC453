#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

void closepipe(int close_fd) {                                                      /* Closes pipe helper function */
   if(close(close_fd)) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
}

void newpipe(int *fd) {                                                             /* Creates new pipe */
   if(pipe(fd)) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
}

void closePipes(int *newfd, int next_read, int *oldfd, int i, int process) {        /* Closes pipes */
   newfd[0] = next_read;
   newfd[1] = oldfd[1];
   closepipe(oldfd[0]);
   if (i == (process)) {
      newfd[1] = 1;
      closepipe(oldfd[1]);
   }
}

void parentHandle(int *next_read, int *oldfd) {                                     /* Handles parent processes */
   if (*next_read)
      closepipe(*next_read);
   closepipe(oldfd[1]);
   *next_read = oldfd[0];
}

int childCheck(int *pid, int *next_read, int *oldfd) {                              /* Checks children processes */
   if ((*pid=fork()) != 0) {
      parentHandle(next_read, oldfd);
      return 0;
   }
   return 1;
}

int pipeline(int processes, int FD[], int process[]) {                              /* Creates a pipeline */
   int oldfd[2], newfd[2], next_read=0, i=0, pid=0;
   
   for (i=1; i<=processes; i++) {
      newpipe(oldfd); 
      if (childCheck(&pid, &next_read, oldfd)) {
         closePipes(newfd, next_read, oldfd, i, processes);
         FD[0] = newfd[0];
         FD[1] = newfd[1];
         process[0] = (i-1);
         break;
      }
   }
   return pid;
}
