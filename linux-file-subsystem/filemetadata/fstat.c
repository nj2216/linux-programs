#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdlib.h>

void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    //File type
    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISFIFO(mode)) perms[0] = 'p';
    else if (S_ISSOCK(mode)) perms[0] = 's';

    //Owner
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    //Group
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    //Other
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';

    printf("Permissions: %s (%o)\n", perms, mode & 07777);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    struct stat sb;
    if (stat(argv[1], &sb) == -1) {
        perror("stat");
        return 1;
    }

    struct passwd *pw = getpwuid(sb.st_uid);
    struct group *gr = getgrgid(sb.st_gid);

    printf("File        : %s\n", argv[1]);
    printf("Inode       : %lu\n", (unsigned long) sb.st_ino);
    print_permissions(sb.st_mode);
    printf("Links       : %lu\n", (unsigned long) sb.st_nlink);
    printf("Owner       : %s (%d)\n", pw ? pw->pw_name : "?", sb.st_uid);
    printf("Group       : %s (%d)\n", gr ? gr->gr_name : "?", sb.st_gid);
    printf("Size        : %lld bytes\n", (long long) sb.st_size);
    printf("Blocks      : %lld (of %d bytes each)\n", (long long) sb.st_blocks, sb.st_blksize);
    printf("Accessed    : %s", ctime(&sb.st_atime));
    printf("Modified    : %s", ctime(&sb.st_mtime));
    printf("Changed     : %s", ctime(&sb.st_ctime));

    return 0;
}