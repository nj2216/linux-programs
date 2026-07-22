#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_JOBS 10
typedef struct {pid_t pid; char cmd[40]; int active; } Job;

Job jobs[MAX_JOBS];

void handle_sigchild(int sig) {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    for(int i = 0; i < MAX_JOBS; i++) {
      if (jobs[i].active && jobs[i].pid == pid) {
        jobs[i].active = 0;
        printf("\n[Background Job %d (%s) finished]\nseashell> ", pid, jobs[i].cmd);
        fflush(stdout);
        break;
      }
    }
  }
}

int main() {
  struct sigaction sa;
  sa.sa_handler = handle_sigchild;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);

  // Initialize jobs array
  for (int i = 0; i < MAX_JOBS; i++) {
    jobs[i].active = 0;
  }

  // Main loop for the shell
  while (1) {
    printf("seashell> ");
    fflush(stdout);
    char input[100];
    if (fgets(input, sizeof(input), stdin) == NULL) {
      break; // Exit on EOF
    }

    // Remove newline character from input
    input[strcspn(input, "\n")] = '\0';

    // Check for background execution
    int background = 0;
    if (input[strlen(input) - 1] == '&') {
      background = 1;
      input[strlen(input) - 1] = '\0'; // Remove '&' from command
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      continue;
    } else if (pid == 0) {
      // Child process
      execlp(input, input, NULL);
      perror("exec failed");
      exit(EXIT_FAILURE);
    } else {
      // Parent process
      if (background) {
        // Add job to jobs array
        for (int i = 0; i < MAX_JOBS; i++) {
          if (!jobs[i].active) {
            jobs[i].pid = pid;
            strncpy(jobs[i].cmd, input, sizeof(jobs[i].cmd) - 1);
            jobs[i].cmd[sizeof(jobs[i].cmd) - 1] = '\0'; // Ensure null-termination
            jobs[i].active = 1;
            printf("[Background Job %d (%s)]\n", pid, jobs[i].cmd);
            break;
          }
        }
      } else {
        waitpid(pid, NULL, 0); // Wait for foreground job to finish
      }
    }
  }

  return 0;
}