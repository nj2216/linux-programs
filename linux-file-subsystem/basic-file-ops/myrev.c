#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2, "usage: myrev <file>\n", 21);
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) { perror("open"); exit(1); }

    struct stat st;
    if (fstat(fd,&st) < 0) { perror("fstat"); exit(1); }

    off_t size = st.st_size;
    off_t left = 0;
    off_t right = size - 1;

    char a, b;

    while (left < right) {
        lseek(fd, left, SEEK_SET);
        read(fd, &a, 1);

        lseek(fd, right, SEEK_SET);
        read(fd, &b, 1);

        lseek(fd, left, SEEK_SET);
        write(fd, &b, 1);

        lseek(fd, right, SEEK_SET);
        write(fd, &a, 1);

        left++;
        right--;
    }
    close(fd);
    return 0;
}