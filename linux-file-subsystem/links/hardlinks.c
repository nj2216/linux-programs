#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

void show_inode_info(const char *label, const char *path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        printf("%-20s : (no such file)\n", label);
        return;
    }
    printf("%-20s : inode=%lu  link_count=%lu\n",
           label, (unsigned long) sb.st_ino, (unsigned long) sb.st_nlink);
}

int main() {
    const char *original = "original.txt";
    const char *hardlink  = "hardlink.txt";

    // Create the original file
    FILE *f = fopen(original, "w");
    if (!f) { perror("fopen"); exit(1); }
    fprintf(f, "hello from original\n");
    fclose(f);

    printf("=== Before creating hard link ===\n");
    show_inode_info("original.txt", original);

    // Create a hard link: new directory entry, SAME inode
    if (link(original, hardlink) == -1) {
        perror("link");
        exit(1);
    }

    printf("\n=== After link() ===\n");
    show_inode_info("original.txt", original);
    show_inode_info("hardlink.txt", hardlink);
    // Notice: same inode number, link_count is now 2

    // Delete one name
    if (unlink(original) == -1) {
        perror("unlink");
        exit(1);
    }

    printf("\n=== After unlink(\"original.txt\") ===\n");
    show_inode_info("original.txt", original);   // gone
    show_inode_info("hardlink.txt", hardlink);   // still there, link_count now 1, data intact

    // Clean up
    unlink(hardlink);

    return 0;
}