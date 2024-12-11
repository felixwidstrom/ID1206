#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

DIR *dir;
struct dirent *dp;
struct dirent **entries = NULL;

#define ARRAY_SIZE (1 << 30) // 1 GiB

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

void write_data(int nio, int nbuf) { // Assignment 2
    int i;

    char** data = (char**)malloc((ARRAY_SIZE/nio) * sizeof(char*));

    char buffer[nbuf];

    FILE* file = fopen("output.txt", "w");
    if(file == NULL) {
        perror("fopen error");
        exit(1);
    }

    for(i = 0; i < ARRAY_SIZE/nio; i++){ 
        data[i] = (char*)malloc(nio);
        if (data[i] == NULL) {
            perror("malloc failed");
            exit(1);
        }   
        memset(data[i], '@', nio);
    }

    if (setvbuf(file, buffer, _IOFBF, sizeof(buffer)) !=0) {
        perror("Buffer error");
        exit(1);
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (i = 0; i < ARRAY_SIZE/nio; i++) {
        fputs(data[i], file);
    }

    gettimeofday(&end, NULL);

    double time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("nio = %d, nbuf = %d, I/O time = %.6f seconds\n", nio, nbuf, time);

    fclose(file);

    // Free up the allocated memory
    for(i = 0; i < ARRAY_SIZE/nio; i++) {
        free(data[i]);
    }
    free(data);
}

int main(int argc, char const *argv[])
{
    files("test");

    // Assignment 2
    const int nio_min = (1 << 10);
    const int nio_max = (1 << 15);
    const int nbuf_min = (1 << 10);
    const int nbuf_max = (1 << 15);


    for(int nio = nio_min; nio <= nio_max; nio <<= 1) {
        for(int nbuf = nbuf_min; nbuf <= nbuf_max; nbuf <<= 1) {
            write_data(nio, nbuf);
        }
    }

    return 0;
}
