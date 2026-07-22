#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 100
#define MAX_ARGS 20
#define MAX_CMDS 10

static void strip_matching_quotes(char *token) {
    size_t len = strlen(token);

    if (len < 2) {
        return;
    }

    if ((token[0] == '"' && token[len - 1] == '"') ||
        (token[0] == '\'' && token[len - 1] == '\'')) {
        memmove(token, token + 1, len - 2);
        token[len - 2] = '\0';
    }
}

static int parse_segment(char *segment, char *argv[]) {
    int argc = 0;

    argv[argc] = strtok(segment, " ");
    while (argv[argc] != NULL && argc < MAX_ARGS - 1) {
        strip_matching_quotes(argv[argc]);
        argv[++argc] = strtok(NULL, " ");
    }

    return argc;
}

int main() {
    char line[MAX_LINE];
    char *segments[MAX_CMDS];
    char *cmd_argv[MAX_CMDS][MAX_ARGS];
    pid_t pids[MAX_CMDS];

    while(1) {
        printf("seashell> ");
        fflush(stdout);
        if(!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = '\0';

        for (int i = 0; i < MAX_CMDS; i++) {
            pids[i] = -1;
        }

        int cmd_count = 0;
        segments[cmd_count] = strtok(line, "|");
        while (segments[cmd_count] != NULL && cmd_count < MAX_CMDS - 1) {
            segments[++cmd_count] = strtok(NULL, "|");
        }

        if (segments[0] == NULL)
            continue;

        int invalid_pipeline = 0;
        for (int i = 0; i < cmd_count; i++) {
            int argc = parse_segment(segments[i], cmd_argv[i]);
            if (argc == 0) {
                invalid_pipeline = 1;
                break;
            }
        }

        if (invalid_pipeline) {
            fprintf(stderr, "Invalid pipeline: empty command between pipes\n");
            continue;
        }

        if (cmd_count == 1 && strcmp(cmd_argv[0][0], "exit") == 0)
            break;

        int prev_read_end = -1;

        for (int i = 0; i < cmd_count; i++) {
            int pipefd[2] = {-1, -1};

            if (i < cmd_count - 1) {
                if (pipe(pipefd) < 0) {
                    perror("pipe");
                    invalid_pipeline = 1;
                    break;
                }
            }

            pids[i] = fork();
            if (pids[i] == 0) {
                if (prev_read_end != -1) {
                    dup2(prev_read_end, STDIN_FILENO);
                    close(prev_read_end);
                }

                if (i < cmd_count - 1) {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                }

                if(execvp(cmd_argv[i][0], cmd_argv[i]) < 0) {
                    perror("Binary Not Exist");
                    exit(127);
                }
            } else if (pids[i] < 0) {
                perror("fork");
                invalid_pipeline = 1;
                if (pipefd[0] != -1) close(pipefd[0]);
                if (pipefd[1] != -1) close(pipefd[1]);
                break;
            }

            if (prev_read_end != -1) {
                close(prev_read_end);
            }

            if (i < cmd_count - 1) {
                close(pipefd[1]);
                prev_read_end = pipefd[0];
            }
        }

        if (prev_read_end != -1) {
            close(prev_read_end);
        }

        int launched = invalid_pipeline ? 0 : cmd_count;
        if (invalid_pipeline) {
            for (int i = 0; i < cmd_count; i++) {
                if (pids[i] > 0) {
                    launched = i + 1;
                }
            }
        }

        for (int i = 0; i < launched; i++) {
            int status;
            waitpid(pids[i], &status, 0);

            if (i == launched - 1 && (status & 0x7F) == 0) {
                int exit_code = (status >> 8) & 0xFF;
                printf("[Process exited with status %d]\n", exit_code);
            } else if (i == launched - 1) {
                printf("[Process terminated abnormally]\n");
            }
        }
    }
    return 0;
}