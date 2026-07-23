#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

long dir_count = 0;
long file_count = 0;

void walk(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat sb;
        if (lstat(fullpath, &sb) == -1) {
            perror(fullpath);
            continue;
        }

        if (S_ISDIR(sb.st_mode)) {
            dir_count++;
            walk(fullpath);
        } else {
            file_count++;
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";
    walk(path);
    printf("Directories: %ld\n", dir_count);
    printf("Files: %ld\n", file_count);
    return 0;
}