#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pipeline.h"

#define MAX_LINE 80
#define MAXCOMMS 21
#define MAXARGS 11

typedef struct {                                                                   /* Command struct */
   char *argv[MAXARGS];
   char *inputFile;
   char *outputFile;
} Command;

int wait_flag = 1;                                                                 /* Flag to determine whether the parent and child should run concurrently */

int check_arg(char **tok) {                                                        /* This function checks for specific flags and makes sure they're valid */
   char *exit_string = "exit";

   if (*tok == NULL)
      return 0;
   if (!strcmp(*tok, exit_string))
      exit(EXIT_SUCCESS);
   if (!strcmp(*tok, "&")) {
      wait_flag = 0;
      *tok = NULL;
      return 0;
   }
   if (!strcmp(*tok, "|")) {
      *tok = strtok(NULL, " ");
      if (!*tok) {
         fprintf(stderr, "cshell: Invalid pipe\n");
         return 1;
      }
      return 0;
   }
   return 1;
}

int nested_parse(int *num_args, char **tok, Command comm_arr[], int com_index) {   /* This would parse the arguments and make sure they are valid */
   if (!*tok) {
      return 1;
   }
   if (*num_args > 10) {
      fprintf(stderr, "cshell: %s: Too many arguments\n", 
         comm_arr[com_index].argv[0]);
      return 1;
   }
   if (!strcmp(*tok, ">")) {                                                       /* If ">" set output file */
      *tok = strtok(NULL, " ");
      if (!*tok) {
         fprintf(stderr, "cshell: Syntax error\n");
         return 1;
      }
      comm_arr[com_index].outputFile = *tok;
      *tok = strtok(NULL, " ");
      return 0;
   }
   if (!strcmp(*tok, "<")) {                                                       /* If "<" set input file */
      *tok = strtok(NULL, " ");
      if (!*tok) {
         fprintf(stderr, "cshell: Syntax error\n");
         return 1;
      }
      comm_arr[com_index].inputFile = *tok;
      *tok = strtok(NULL, " ");
      return 0;
   }
   comm_arr[com_index].argv[(*num_args)++] = *tok;
   *tok = strtok(NULL, " ");
   return 0;
}

int parse_args(Command comm_arr[], char buf[], int *num_args) {                    /* Higher level parsing, makes sure pipe is valid */
   char *tok = strtok(buf, " ");
   int com_index=0;
   if (!strcmp(tok, "|")) {
      fprintf(stderr, "cshell: Invalid pipe\n");
      return -1;
   }
   while (tok) {                                                                    
      *num_args=0;
      comm_arr[com_index].outputFile = NULL;
      comm_arr[com_index].inputFile = NULL;
      while (check_arg(&tok)) {                                                    /* While there are arguments, check each argument */
         if (nested_parse(num_args, &tok, comm_arr, com_index))
            return -1;
      }
      comm_arr[com_index++].argv[*num_args] = NULL;
   }
   return com_index;
}

int execute_child(Command command, int inFD, int outFD) {                          /* Executes the child */
   dup2(inFD, STDIN_FILENO);
   dup2(outFD, STDOUT_FILENO);
   if (execvp(command.argv[0], command.argv) == -1) {
      fprintf(stderr, "cshell: %s: Command not found\n", command.argv[0]);
      exit(EXIT_FAILURE);
   }
   return 0;
}

void child_helper(char *files[], int rw[], int FD[], Command comm_arr[], 
   int process[]) {                                                                /* Handles opening and closing pipes */
   if (files[1]) {
      if ((rw[1] = open(files[1],O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) {
         fprintf(stderr, "cshell: Unable to open file for output\n");
         exit(EXIT_FAILURE);
      }
      if (FD[1] != 1)
         closepipe(FD[1]);
      FD[1] = rw[1];
   }
   if (files[0]) {
      if ((rw[0] = open(files[0],O_RDONLY)) == -1) {
         fprintf(stderr, "cshell: Unable to open file for input\n");
         exit(EXIT_FAILURE);
      }
      if (FD[0] != 0)
         closepipe(FD[0]);
      FD[0] = rw[0];
   }
   execute_child(comm_arr[process[0]], FD[0], FD[1]);
}

int child_check(int FD[], Command comm_arr[], int *process) {                      /* Checks validity of the child */
   int rw[2];
   char *files[2];
   files[0] = comm_arr[process[0]].inputFile;
   files[1] = comm_arr[process[0]].outputFile;
   if (!process[1])
      child_helper(files, rw, FD, comm_arr, process);
   return 0;
}

int process_check(char *buf, int process[], int FD[]) {                            /* Check whether the process being executed is valid */
   if ((int)strlen(buf) > 1023) {
      fprintf(stderr, "cshell: Command line too long\n");
      while (getchar() != '\n');
      printf("osh>");
      return 1;
   }
   if (process[3] == -1) {
      printf("osh>");
      return 1;
   }
   if (process[3] > 20) {
      fprintf(stderr, "cshell: Too many commands\n");
      printf("osh>");
      return 1;
   }
   if (process[3] == 1) {
      if ((process[1] = fork()) == -1) {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
   }
   else if (process[3] > 1)
      process[1] = pipeline(process[3], FD, process);
   return 0;
}

int init(char *buf, int process[], int FD[]) {                                     /* Initialize the buffers and file descriptors */
   if (feof(stdin) || !strcmp(buf,"\n")) {
      printf("osh>");
      return 1;
   }
   process[0] = 0;
   FD[0]=0;
   FD[1]=1;
   if (buf[strlen(buf) - 1] == '\n')
      buf[strlen(buf) - 1] = 0;
   return 0;
}

int main(void) {                                                           
   char buf[MAX_LINE];
   char prev_buf[MAX_LINE] = "";
   Command comm_arr[MAXCOMMS];
   int status, process[4], FD[2], num_args=0;
   int should_run = 1;
   setbuf(stdout, NULL);
   printf("osh>");
   while (should_run) {                                                            /* Keeps shell running until user exits */
      fgets(buf, MAX_LINE, stdin);                                                 /* Run previous command */
      if (!strcmp("!!\n", buf)) {                                                  /* If buffer is empty, there are no commands in history */
         if (!strcmp(prev_buf, "")) {
            fprintf(stderr, "No commands in history.");
            printf("\nosh>");
            continue;
         }
         strcpy(buf, prev_buf);                                                    /* Set previous buffer to current buffer because it was re-run */
      }
      strcpy(prev_buf, buf);                                                       /* Set previous buffer for next iteration */
      if (init(buf, process, FD))
         continue;
      process[3] = parse_args(comm_arr, buf, &num_args);
      if (process_check(buf, process, FD))
         continue;
      if (child_check(FD, comm_arr, process))
         continue;
      if (wait_flag == 1) {                                                        /* Wait for child unless otherwise specified */
         if ((process[1] = waitpid(process[1], &status, 0)) < 0) {
            perror(NULL);
            exit(EXIT_FAILURE);
         }
      } else {
         wait_flag = 1;
      }
      printf("osh>");
   }
   printf("exit\n");
   exit(EXIT_SUCCESS);
}
