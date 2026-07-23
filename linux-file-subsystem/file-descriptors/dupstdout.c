#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("output.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("open"); exit(1); }

    int saved_stdout = dup(STDOUT_FILENO); // copy of the real stdout to restore later

    if (dup2(fd, STDOUT_FILENO) == -1) { perror("dup2"); exit(1); }
    close(fd);

    printf("This goes into output.log, not the terminal!\n");
    fflush(stdout);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    printf("This goes back to the terminal.\n");

    return 0;
}