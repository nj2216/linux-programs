#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

char largest_path[4096] = "";
off_t largest_size = -1;

void walk(const char *path) {
    DIR *dir = opendir(path);
    if(!dir) {
        perror(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat sb;
        if (lstat(fullpath, &sb) == -1) {
            perror(fullpath);
            continue;
        }

        if (S_ISDIR(sb.st_mode)) {
            walk(fullpath);
        } else if (S_ISREG(sb.st_mode)) {
            if (sb.st_size > largest_size) {
                largest_size = sb.st_size;
                strncpy(largest_path, fullpath, sizeof(largest_path) - 1);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";
    walk(path);

    if (largest_size >= 0)
        printf("Largest file: %s (%lld bytes)\n", largest_path, (long long) largest_size);
    else
        printf("No regular files found.\n");

    return 0;
}
