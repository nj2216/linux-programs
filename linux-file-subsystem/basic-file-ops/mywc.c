#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2,"usage: mywc <file>\n", 20);
        exit(0);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); exit(1); }

    char buf[BUF_SIZE];
    ssize_t n;

    long bytes = 0, lines = 0, words = 0;
    int in_word = 0; // state ACCROSS chunk bounds
    while ((n = read(fd, buf, BUF_SIZE)) > 0) {
        bytes += n;
        for (ssize_t i = 0; i < n; i++) {
            char c = buf[i];
            if (c == '\n') lines++;

            if (c == ' ' || c == '\n' || c == '\t') {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;
            }
        }
    }

    if (n < 0) { perror("read"); exit(1); }

    printf("%7ld %7ld %7ld %s\n", lines, words, bytes, argv[1]);

    close(fd);
    return 0;
}