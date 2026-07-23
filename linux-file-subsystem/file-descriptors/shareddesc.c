#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd1 = open("data.txt", O_RDONLY);
    if (fd1 == -1) { perror("open"); return 1; }

    int fd2 = dup(fd1);

    char buf[16];
    read(fd1, buf, 10);

    off_t off1 = lseek(fd1, 0, SEEK_CUR);
    off_t off2 = lseek(fd2, 0, SEEK_CUR);

    printf("fd1 offset: %ld\n", (long) off1);
    printf("fd2 offset: %ld\n", (long) off2);

    close(fd1);
    close(fd2);

    return 0;
}