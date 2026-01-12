// barrier_sem.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

typedef struct barrier {
    int n;
    int arrived;
    pthread_mutex_t mtx;
    sem_t sem;
} barrier_t;

static barrier_t B;

static void barrier_init(barrier_t *b, int n)
{
    b->n = n;
    b->arrived = 0;

    if (pthread_mutex_init(&b->mtx, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
    if (sem_init(&b->sem, 0, 0) != 0) {
        perror("sem_init");
        exit(1);
    }
}

static void barrier_destroy(barrier_t *b)
{
    if (sem_destroy(&b->sem) != 0) perror("sem_destroy");
    if (pthread_mutex_destroy(&b->mtx) != 0) perror("pthread_mutex_destroy");
}

static void barrier_point(barrier_t *b)
{
    pthread_mutex_lock(&b->mtx);
    b->arrived++;

    if (b->arrived == b->n) {
        // last thread releases everyone else
        for (int i = 0; i < b->n - 1; i++)
            sem_post(&b->sem);

        pthread_mutex_unlock(&b->mtx);
        return; // last thread passes immediately
    }

    pthread_mutex_unlock(&b->mtx);

    // others wait
    while (sem_wait(&b->sem) == -1 && errno == EINTR) {
        // retry if interrupted by a signal
    }
}

static void *tfun(void *v)
{
    int tid = *(int *)v;
    free(v);

    usleep((rand() % 200) * 1000);

    printf("%d reached the barrier\n", tid);
    fflush(stdout);

    barrier_point(&B);

    printf("%d passed the barrier\n", tid);
    fflush(stdout);

    return NULL;
}

#define NTHRS 5

int main(void)
{
    srand(12345);

    printf("NTHRS=%d\n", NTHRS);
    barrier_init(&B, NTHRS);

    pthread_t th[NTHRS];
    for (int i = 0; i < NTHRS; i++) {
        int *tid = malloc(sizeof(*tid));
        if (!tid) { perror("malloc"); return 1; }
        *tid = i;

        if (pthread_create(&th[i], NULL, tfun, tid) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < NTHRS; i++)
        pthread_join(th[i], NULL);

    barrier_destroy(&B);
    return 0;
}

