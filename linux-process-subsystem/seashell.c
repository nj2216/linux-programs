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

char* trim_whitespace(char* text) {
    while (*text == ' ' || *text == '\t') {
        text++;
    }

    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\t')) {
        text[--len] = '\0';
    }

    return text;
}

int split_args(char* cmd, char* args[], int max_args) {
    int argc = 0;
    char* src = cmd;
    char* token_start = NULL;
    char* dst = cmd;
    char quote = '\0';

    while (*src != '\0') {
        if (quote == '\0' && (*src == ' ' || *src == '\t')) {
            if (token_start != NULL) {
                *dst++ = '\0';
                token_start = NULL;
                if (argc >= max_args - 1) {
                    break;
                }
            }
            src++;
            continue;
        }

        if (*src == '\'' || *src == '"') {
            if (quote == '\0') {
                quote = *src;
                if (token_start == NULL) {
                    token_start = dst;
                    args[argc++] = token_start;
                }
            } else if (*src == quote) {
                quote = '\0';
            } else {
                if (token_start == NULL) {
                    token_start = dst;
                    args[argc++] = token_start;
                }
                *dst++ = *src;
            }
            src++;
            continue;
        }

        if (token_start == NULL) {
            token_start = dst;
            args[argc++] = token_start;
            if (argc >= max_args) {
                break;
            }
        }

        *dst++ = *src++;
    }

    if (quote != '\0') {
        fprintf(stderr, "unterminated quote\n");
        args[0] = NULL;
        return 0;
    }

    if (token_start != NULL) {
        *dst++ = '\0';
    }

    args[argc] = NULL;
    return argc;
}

int split_pipeline(char* line, char* cmds[], int max_cmds) {
    int count = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;
    char* start = line;

    while (*line != '\0' && count < max_cmds) {
        if (*line == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
        } else if (*line == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
        } else if (*line == '|' && !in_single_quote && !in_double_quote) {
            *line = '\0';
            cmds[count++] = trim_whitespace(start);
            start = line + 1;
        }
        line++;
    }

    if (count < max_cmds) {
        cmds[count++] = trim_whitespace(start);
    }

    return count;
}

int execute_line(char* line);

int execute_builtin(char* cmd) {
    if (strcmp(cmd, "exit") == 0) {
        return -1;
    }

    if (strcmp(cmd, "jobs") == 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].active) {
                printf("[%d] %s\n", jobs[i].pid, jobs[i].cmd);
            }
        }
        return 0;
    }

    if (strcmp(cmd, "fg") == 0) {
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
                    return exit_code;
                }
                printf("[Process terminated abnormally]\n");
                return 1;
            }
        }

        if (!found) {
            printf("No background jobs to bring to foreground.\n");
        }

        return 0;
    }

    return -2;
}

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

int run_cmd(char* cmd) {
    char* args[MAX_ARGS];
    char cmd_copy[40];

    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    int argc = split_args(cmd, args, MAX_ARGS);
    if (argc == 0 || args[0] == NULL) {
        return 0;
    }

    int background = 0;
    if (argc > 0 && strcmp(args[argc - 1], "&") == 0) {
        background = 1;
        args[argc - 1] = NULL;
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
                return exit_code;
            } else {
                printf("[Process terminated abnormally]\n");
                return 1;
            }
        } else {
            register_background_job(pid, cmd_copy);
            return 0;
        }
    } else {
        perror("fork");
        return 1;
    }

    return 0;
}

int run_pipeline(char* line) {
    char* cmds[MAX_CMDS];
    int cmd_count = split_pipeline(line, cmds, MAX_CMDS);

    if (cmd_count == 1) {
        int builtin_status = execute_builtin(cmds[0]);
        if (builtin_status != -2) {
            return builtin_status;
        }
        return run_cmd(cmds[0]);
    }

    pid_t pids[MAX_CMDS];
    int prev_read_end = -1;
    int launch_count = 0;
    int pipeline_failed = 0;
    int last_status = 0;

    for (int i = 0; i < cmd_count; i++) {
        int pipefd[2] = {-1, -1};

        if (cmds[i][0] == '\0') {
            fprintf(stderr, "invalid null command\n");
            return 2;
        }

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
            int argc = split_args(cmds[i], args, MAX_ARGS);
            if (argc == 0 || args[0] == NULL) {
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
        int status;
        waitpid(pids[i], &status, 0);
        if (i == launch_count - 1) {
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else {
                last_status = 1;
            }
        }
    }

    if (pipeline_failed) {
        fprintf(stderr, "pipeline setup failed\n");
        return 1;
    }

    return last_status;
}

int execute_group(char* text) {
    char* body = trim_whitespace(text);
    size_t len = strlen(body);

    if (len >= 2 && body[0] == '{' && body[len - 1] == '}') {
        body[len - 1] = '\0';
        body = trim_whitespace(body + 1);
    }

    return execute_line(body);
}

int execute_line(char* line) {
    char* cursor = trim_whitespace(line);
    int last_status = 0;

    while (*cursor != '\0') {
        int in_single_quote = 0;
        int in_double_quote = 0;
        int brace_depth = 0;
        char* segment_start = cursor;
        char* op = NULL;
        int op_type = 0;

        while (*cursor != '\0') {
            if (*cursor == '\'' && !in_double_quote) {
                in_single_quote = !in_single_quote;
            } else if (*cursor == '"' && !in_single_quote) {
                in_double_quote = !in_double_quote;
            } else if (!in_single_quote && !in_double_quote) {
                if (*cursor == '{') {
                    brace_depth++;
                } else if (*cursor == '}' && brace_depth > 0) {
                    brace_depth--;
                } else if (brace_depth == 0) {
                    if (*cursor == ';') {
                        op = cursor;
                        op_type = 1;
                        break;
                    }
                    if (cursor[0] == '&' && cursor[1] == '&') {
                        op = cursor;
                        op_type = 2;
                        break;
                    }
                }
            }
            cursor++;
        }

        if (op != NULL) {
            *op = '\0';
        }

        char* segment = trim_whitespace(segment_start);
        if (*segment != '\0') {
            int status;
            if (segment[0] == '{') {
                status = execute_group(segment);
            } else {
                status = run_pipeline(segment);
            }

            if (status == -1) {
                return -1;
            }
            last_status = status;
        }

        if (op == NULL) {
            break;
        }

        if (op_type == 1) {
            cursor = op + 1;
        } else {
            cursor = op + 2;
            if (last_status != 0) {
                while (*cursor != '\0') {
                    int in_sq = 0;
                    int in_dq = 0;
                    int depth = 0;

                    char* skip = cursor;
                    while (*skip != '\0') {
                        if (*skip == '\'' && !in_dq) {
                            in_sq = !in_sq;
                        } else if (*skip == '"' && !in_sq) {
                            in_dq = !in_dq;
                        } else if (!in_sq && !in_dq) {
                            if (*skip == '{') {
                                depth++;
                            } else if (*skip == '}' && depth > 0) {
                                depth--;
                            } else if (depth == 0 && *skip == ';') {
                                cursor = skip + 1;
                                break;
                            }
                        }
                        skip++;
                    }

                    if (*skip == '\0') {
                        return last_status;
                    }
                    break;
                }
            }
        }

        cursor = trim_whitespace(cursor);
    }

    return last_status;
}

int main() {
    char line[MAX_LINE];
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

        if (line[0] == '\0') {
            continue;
        }

        if (execute_line(line) == -1) {
            break;
        }
    }
    return 0;
}