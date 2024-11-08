#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>

#define N 2048
#define NTHRD 2

void pairs() {
    const char *fifo = "/tmp/fifo";
    const char *msg = "This is a message!";
    const int size = 256;

    char buffer[size];
    int fd;

    pid_t pid;
    
    pid = fork();
    
    if (pid == -1) {
        printf("Error: Failed to fork");
    } else if (pid > 0) {
        mkfifo(fifo, 0666);
        fd = open(fifo, O_WRONLY);
        write(fd, msg, strlen(msg) + 1);
        close(fd);
        wait(NULL);
        unlink(fifo);
    } else {
        fd = open(fifo, O_RDONLY);
        read(fd, buffer, size);
        close(fd);

        printf("Received: \"%s\"\n", buffer);

        _exit(16);
    }

    return;
}

typedef struct { int imin; int imax; double psum; } args_t;
double *a;

void *partial_sum(void *p) {
    args_t *args = (args_t *) p;
    args->psum = 0.0;
    for (int i = args->imin; i < args->imax; i++)
        args->psum += a[i];
}

void performance() {
    pthread_t thrd[NTHRD];
    args_t thrd_args[NTHRD];

    // Create input array
    a = (double *) malloc(N * sizeof(double));
    for (int i = 0; i < N; i++)
        a[i] = i;

    // Create threads
    for (int i = 0; i < NTHRD; i++) {
        thrd_args[i].imin = i * N / NTHRD;
        thrd_args[i].imax = (i + 1) * N / NTHRD;
        if (pthread_create(&thrd[i], NULL, partial_sum, &thrd_args[i])) {
            perror("pthread_create");
            exit(1);
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < NTHRD; i++)
        if (pthread_join(thrd[i], NULL)) {
            perror("pthread_join");
            exit(1);
        }

    // Compute global sum
    double gsum = 0.0;
    for (int i = 0; i < NTHRD; i++)
        gsum += thrd_args[i].psum;

    printf("gsum = %.1f\n", gsum);
}

void queue() {
    const char *queue = "/mqueue";
    const char *msg = "status";
    const int size = 256;

    char buffer[size];
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = size;
    ssize_t n;
    mqd_t mqd;

    pid_t pid;

    pid = fork();

    if (pid == -1) {
        printf("Error: Failed to fork");
    } else if (pid > 0) {
        sleep(1);
        mqd = mq_open(queue, O_RDWR);
        if (mqd == -1) printf("Error: Failed to open message queue");
        mq_send(mqd, msg, strlen(msg), 0);
        n = mq_receive(mqd, buffer, size, 0);
        buffer[n] = '\0';
        mq_close(mqd);
        wait(NULL);
        printf("Received: \"%s\"", buffer);
        mq_unlink(queue);
    } else {
        mqd = mq_open(queue, O_RDWR|O_CREAT, 0666, &attr);
        if (mqd == -1) printf("Error: Failed to create message queue");
        n = mq_receive(mqd, buffer, size, 0);
        buffer[n] = '\0';
        if (buffer == "status") {
            mq_send(mqd, "OK", strlen("OK"), 0);
        }
        mq_close(mqd);

        printf("Received: \"%s\"", buffer);

        _exit(16);
    }

    return;
}

int main(int argc, char const *argv[])
{
    // pairs();
    // struct timespec start, end;
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    // for (int i = 0; i < 1000; i++) {
    //     performance();
    // }
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
    // printf("Time (ns): %ld", (1000000000 * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)) / 1000);
    queue();

    return 0;
}