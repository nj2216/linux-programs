#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void mode_to_str(mode_t mode, char *out) {
    strcpy(out, "----------");

    if (S_ISDIR(mode)) out[0] = 'd';
    else if (S_ISLNK(mode)) out[0] = 'l';

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    struct stat sb;
    char fullpath[1024];
    char perms[11];
    char timebuf[64];

    while((entry = readdir(dir)) != NULL) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &sb) == -1) {
            perror("lstat");
            continue;
        }

        mode_to_str(sb.st_mode, perms);

        struct passwd *pw = getpwuid(sb.st_uid);
        struct group *gr = getgrgid(sb.st_gid);

        struct tm *tm_info = localtime(&sb.st_mtime);
        strftime(timebuf, sizeof(timebuf),"%b %d %H:%M", tm_info);

        printf("%s %3lu %-8s %-8s %8lld %s %s\n",
       perms,
       (unsigned long) sb.st_nlink,
       pw ? pw->pw_name : "?",
       gr ? gr->gr_name : "?",
       (long long) sb.st_size,
       timebuf,
       entry->d_name);
    }

    closedir(dir);
    return 0;
}