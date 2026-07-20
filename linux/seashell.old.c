    #define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE 100
#define MAX_ARGS 20
#define MAX_CMDS 10
#define MAX_JOBS 10

typedef struct {pid_t pid; char cmd[40]; int active; } Job;
Job jobs[MAX_JOBS];
volatile sig_atomic_t child_exit_pending = 0;

void handle_sigchild(int sig) {
    (void)sig;

    child_exit_pending = 1;
}

void init_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].active = 0;
        jobs[i].pid = 0;
        jobs[i].cmd[0] = '\0';
    }
}

void register_background_job(pid_t pid, const char* cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].pid = pid;
            strncpy(jobs[i].cmd, cmd, sizeof(jobs[i].cmd) - 1);
            jobs[i].cmd[sizeof(jobs[i].cmd) - 1] = '\0';
            jobs[i].active = 1;
            printf("[Background Job %d (%s)]\n", pid, jobs[i].cmd);
            return;
        }
    }

    fprintf(stderr, "job table full; background job %d is untracked\n", pid);
}

void reap_background_jobs(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].active && jobs[i].pid == pid) {
                jobs[i].active = 0;
                printf("\n[Background process %d (%s) completed]\n", pid, jobs[i].cmd);
                break;
            }
        }
    }

    child_exit_pending = 0;
}

void run_cmd(char* cmd) {
    char* args[MAX_ARGS];
    int i = 0;
    char cmd_copy[40];

    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    // Split command into argv tokens.
    args[i] = strtok(cmd, " \t");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        args[++i] = strtok(NULL, " \t");
    }
    if (i == 0 && args[0] == NULL) {
        return;
    }

    int background = 0;
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1;
        args[i - 1] = NULL;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) < 0) {
            perror("Binary Not Exist");
            exit(127);
        }
    } else if (pid > 0) {
        int status;
        if (!background) {
            waitpid(pid, &status, 0);

            if ((status & 0x7F) == 0) {
                int exit_code = (status >> 8) & 0xFF;
                printf("\n[Process exited with status %d]\n\n", exit_code);
            } else {
                printf("[Process terminated abnormally]\n");
            }
        } else {
            register_background_job(pid, cmd_copy);
        }
    } else {
        perror("fork");
    }
}

int main() {
    char line[MAX_LINE];
    char* cmds[MAX_CMDS];
    struct sigaction sa;

    sa.sa_handler = handle_sigchild;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    init_jobs();

    while(1) {
        if (child_exit_pending) {
            reap_background_jobs();
        }

        printf("seashell> ");
        fflush(stdout);
        if(!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = '\0';

        cmds[0] = strtok(line, "|");
        int cmd_count = 0;
        while(cmds[cmd_count] != NULL && cmd_count < MAX_CMDS - 1) {
            cmds[++cmd_count] = strtok(NULL, "|");
        }     

        if (cmds[0] == NULL)
            continue;
        else if (cmd_count == 1 && strcmp(cmds[0], "exit") == 0)
            break;
        else if (cmd_count == 1 && strcmp(cmds[0], "jobs") == 0) {
            for (int i = 0; i < MAX_JOBS; i++) {
                if (jobs[i].active) {
                    printf("[%d] %s\n", jobs[i].pid, jobs[i].cmd);
                }
            }
        }
        else if (cmd_count == 1 && strcmp(cmds[0], "fg") == 0) {
            int found = 0;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (jobs[i].active) {
                    found = 1;
                    printf("Bringing job %d (%s) to foreground\n", jobs[i].pid, jobs[i].cmd);
                    int status;
                    waitpid(jobs[i].pid, &status, 0);
                    jobs[i].active = 0;
                    if ((status & 0x7F) == 0) {
                        int exit_code = (status >> 8) & 0xFF;
                        printf("\n[Process exited with status %d]\n\n", exit_code);
                    } else {
                        printf("[Process terminated abnormally]\n");
                    }
                    break;
                }
            }
            if (!found) {
                printf("No background jobs to bring to foreground.\n");
            }
        }
        else if (cmd_count == 1) {
            run_cmd(cmds[0]);
        } else {
            pid_t pids[MAX_CMDS];
            int prev_read_end = -1;
            int launch_count = 0;
            int pipeline_failed = 0;

            // Build each pipeline stage as a child process and connect FDs.
            for (int i = 0; i < cmd_count; i++) {
                int pipefd[2] = {-1, -1};

                if (i < cmd_count - 1) {
                    if (pipe(pipefd) < 0) {
                        perror("pipe");
                        pipeline_failed = 1;
                        break;
                    }
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork");
                    pipeline_failed = 1;
                    if (pipefd[0] != -1) close(pipefd[0]);
                    if (pipefd[1] != -1) close(pipefd[1]);
                    break;
                }

                if (pid == 0) {
                    if (prev_read_end != -1) {
                        dup2(prev_read_end, STDIN_FILENO);
                    }
                    if (pipefd[1] != -1) {
                        dup2(pipefd[1], STDOUT_FILENO);
                    }

                    if (prev_read_end != -1) close(prev_read_end);
                    if (pipefd[0] != -1) close(pipefd[0]);
                    if (pipefd[1] != -1) close(pipefd[1]);

                    char* args[MAX_ARGS];
                    int a = 0;
                    args[a] = strtok(cmds[i], " \t");
                    while (args[a] != NULL && a < MAX_ARGS - 1) {
                        args[++a] = strtok(NULL, " \t");
                    }

                    if (args[0] == NULL) {
                        fprintf(stderr, "invalid null command\n");
                        exit(2);
                    }

                    execvp(args[0], args);
                    perror("Binary Not Exist");
                    exit(127);
                }

                pids[launch_count++] = pid;

                if (prev_read_end != -1) close(prev_read_end);
                if (pipefd[1] != -1) close(pipefd[1]);
                prev_read_end = pipefd[0];
            }

            if (prev_read_end != -1) close(prev_read_end);

            for (int i = 0; i < launch_count; i++) {
                waitpid(pids[i], NULL, 0);
            }

            if (pipeline_failed) {
                fprintf(stderr, "pipeline setup failed\n");
            }
        }
    }
    return 0;
}