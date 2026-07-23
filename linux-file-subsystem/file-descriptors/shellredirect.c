#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.txt output.txt\n", argv[0]);
        exit(1);
    }

    int out_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) { perror("open output"); exit(1); }

    int saved_stdout = dup(STDOUT_FILENO);

    if (dup2(out_fd, STDOUT_FILENO) == -1) { perror("dup2"); exit(1); }
    close(out_fd);

    int in_fd = open(argv[1], O_RDONLY);
    if (in_fd == -1) {
        perror("open input");
        dup2(saved_stdout, STDOUT_FILENO);
        exit(1);
    }

    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(in_fd, buf, BUF_SIZE)) > 0) {
        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(STDOUT_FILENO, buf + written, n - written);
            if (w == -1) { perror("write"); exit(1); }
            written += w;
        }
    }
    if (n == -1) perror("read");

    close(in_fd);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    return 0;
}