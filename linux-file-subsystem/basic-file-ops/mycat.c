#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2, "usage: mycat <file>\n", 21);
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = read(fd, buf, BUF_SIZE)) > 0) {
        ssize_t written = write(1, buf, n);
        if (written != n) {
            write(2, "write error\n", 12);
            exit(1);
        }
    }

    if (n < 0) {
        perror("read");
        exit(1);
    }

    close(fd);
    return 0;
}