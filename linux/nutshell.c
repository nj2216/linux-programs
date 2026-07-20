#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 100
#define MAX_ARGS 20

int main() {
    char line[MAX_LINE];
    char* args[MAX_ARGS];

    while(1) {
        printf("nutshell> ");
        fflush(stdout);
        if(!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = '\0';

        int i = 0;
        args[i] = strtok(line, " ");
        while(args[i] != NULL && i < MAX_ARGS - 1) {
            args[++i] = strtok(NULL, " ");
        }

        if(args[0] == NULL)
            continue;
        if (strcmp(args[0], "exit") == 0)
            break;
        int background = 0;
        if (strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL;
        }
        pid_t pid = fork();
        if(pid == 0) {
            if(execvp(args[0], args) < 0) {
                perror("Binary Not Exist");
                exit(127);
            }
        } 
        else if (pid > 0) {
            int status;
            if (!background) {
                wait(&status);
                
                if ((status & 0x7F) == 0) {
                    int exit_code = (status >> 8) & 0xFF;
                    printf("[Process exited with status %d]\n", exit_code);
                } else {
                    printf("[Process terminated abnormally]\n");
                }
            }
            else {
                printf("[Process running in background with PID %d]\n", pid);
            }
        }
    }
    return 0;
}