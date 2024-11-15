#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NTHRD 8
#define RADIUS 1
#define SQUARE_AREA (4.0 * RADIUS * RADIUS)
#define CIRCLE_AREA (M_PI * RADIUS * RADIUS)

// Assignment 1
double Nhit = 0.0;
double Ntry = 0.0;

pthread_mutex_t lock;

void* worker(void* arg) {
    struct drand48_data buffer;
    srand48_r((long) pthread_self, &buffer);

    while (1) {
        double x, y;
        drand48_r(&buffer, &x);
        drand48_r(&buffer, &y);
        x = (x * 2.0 - 1.0) * RADIUS;
        y = (y * 2.0 - 1.0) * RADIUS;

        pthread_mutex_lock(&lock);
        Ntry++;
        if ((x * x) + (y * y) <= RADIUS * RADIUS) {
            Nhit++;
        }
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void montecarlo() {
    pthread_t thrds[NTHRD];
    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < NTHRD; i++) {
        thrds[i] = pthread_create(&thrds[i], NULL, worker, NULL);
    }

    while (1) {
        pthread_mutex_lock(&lock);
        double estimate = (Nhit / Ntry) * SQUARE_AREA;
        pthread_mutex_unlock(&lock);

        printf("Estimate: %f\nHits: %f\nTries: %f\nTarget: %f\n\n", estimate, Nhit, Ntry, CIRCLE_AREA);

        sleep(5);
    }
}

// Assignment 2

int counter = 0;
const int nthr = 1000;

typedef struct {
    /* MEMBERS TO BE DEFINED */
    atomic_flag value;
} mysemaphore_t;

mysemaphore_t sem;

int sem_init (mysemaphore_t *s) {
    /* FUNCTION TO BE IMPLEMENTED */
    ATOMIC_VAR_INIT(s);
    return 0;
}

int sem_wait (mysemaphore_t *s) {
    /* FUNCTION TO BE IMPLEMENTED */
    while(atomic_flag_test_and_set(&s->value));
    return 0;
}

int sem_post (mysemaphore_t *s) {
    /* FUNCTION TO BE IMPLEMENTED */
    atomic_flag_clear(&s->value);
    return 0;
}

void* func() {
    sleep(1);
    sem_wait(&sem);
    counter++;
    sem_post(&sem);
}

int main(int argc, char const *argv[])
{
    montecarlo();

    pthread_t thr[nthr];

    sem_init(&sem);

    for (int i = 0; i < nthr; i++)
    pthread_create(&thr[i], NULL, &func , NULL);

    for (int i = 0; i < nthr; i++)
    pthread_join(thr[i], NULL);

    printf("counter = %d\n", counter);

    return 0;
}
