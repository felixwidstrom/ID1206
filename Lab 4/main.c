#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>

DIR *dir;
struct dirent *dp;
struct dirent **entries = NULL;

void files(char *path)
{
    dir = opendir(path);
    entries = malloc(sizeof(struct dirent) * 5);

    int count = 0;

    while ((dp = readdir(dir)) != NULL)
    {
        entries[count] = malloc(sizeof(struct dirent));
        *entries[count] = *dp;
        count++;
    }

    closedir(dir);

    printf("Directory: %s\n", path);
    for (int i = 0; i < count; i++) {
        if (entries[i]->d_type == DT_REG) {
            printf("\tName: %s, Inode: %ld\n", entries[i]->d_name, entries[i]->d_ino);
        }
    }

    for (int i = 0; i < count; i++) {
        if (entries[i]->d_type == DT_DIR) {
            char *name = entries[i]->d_name;
            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                char *new_path = malloc(strlen(path) + strlen(name) + 2);
                strcpy(new_path, path);
                strcat(new_path, "/");
                strcat(new_path, name);
                files(new_path);
                free(new_path);
            }
        }
        free(entries[i]);
    }
}

int main(int argc, char const *argv[])
{
    files("test");
    // buffering();
    return 0;
}
