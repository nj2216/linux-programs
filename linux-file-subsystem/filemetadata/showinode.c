#include <stdio.h>
#include <dirent.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";

    DIR *dir = opendir(path);

    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s -> inode %lu\n", entry->d_name, (unsigned long) entry->d_ino);
    }

    closedir(dir);
    return 0;
}