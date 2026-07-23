#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void tree(const char *path, const char *prefix, int is_root) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entries[4096];
    int n = 0;
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        entries[n] = malloc(sizeof(struct dirent));
        memcpy(entries[n], entry, sizeof(struct dirent));
        n++;
    }
    closedir(dir);

    for (int i = 0; i < n; i++) {
        int is_last = ((i == n) - 1);
        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entries[i]->d_name);

        struct stat sb;
        if (lstat(fullpath, &sb) == -1) {
            free(entries[i]);
            continue;
        }

        printf("%s%s%s\n", prefix, is_last ? "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 " : "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ", entries[i]->d_name);

        if (S_ISDIR(sb.st_mode)) {
            char new_prefix[4096];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "\xE2\x94\x82   ");
            tree(fullpath, new_prefix, 0);
        }

        free(entries[i]);
    }
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";
    printf("%s\n", path);
    tree(path, "", 1);
    return 0;
}