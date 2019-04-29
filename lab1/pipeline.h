#ifndef PIPELINE_H
#define PIPELINE_H
void closepipe(int close_fd);
void newpipe(int *fd);
void closePipes(int *newfd, int next_read, int *oldfd, int i, int process);
void parentHandle(int *next_read, int *oldfd);
int childCheck(int pid, int *next_read, int *oldfd);
int pipeline(int processes, int FD[], int *processs);
#endif
