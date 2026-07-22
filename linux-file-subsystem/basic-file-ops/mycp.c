#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#define BUF_SIZE 8192

ssize_t write_all(int fd, const char *buf, size_t count) {
    size_t total = 0;
    while (total < count) {
        ssize_t n = write(fd, buf + total, count - total);
        if (n < 0) {
            if (errno == EINTR) continue;  // interrupted, retry
            return -1;
        }
        total += n;
    }
    return total;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(2, "usage: mycp <src> <dst>\n", 25);
        exit(1);
    }

    int src = open(argv[1], O_RDONLY);
    if (src < 0) { perror("open src"); exit(1); }

    // O_CREAT|O_TRUNC: create if missing, truncate if exists.
    // 0644 = rw-r--r--, subject to umask.
    int dst = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst < 0) { perror("open dst"); exit(1); }

    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = read(src, buf, BUF_SIZE)) > 0) {
        if (write_all(dst, buf, n) < 0) {
            perror("write");
            exit(1);
        }
    }
    if (n < 0) { perror("read"); exit(1); }

    close(src);
    close(dst);
    return 0;
}