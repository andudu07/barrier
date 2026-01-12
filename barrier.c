// barrier.c
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
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
    pthread_mutex_init(&b->mtx, NULL);
    sem_init(&b->sem, 0, 0);
}

static void barrier_destroy(barrier_t *b)
{
    sem_destroy(&b->sem);
    pthread_mutex_destroy(&b->mtx);
}

static void barrier_point(barrier_t *b)
{
    pthread_mutex_lock(&b->mtx);
    b->arrived++;

    if (b->arrived == b->n) {
        // ultimul thread  ii deblocheaza pe ceilalti (n-1 semnale)
        for (int i = 0; i < b->n - 1; i++) {
            sem_post(&b->sem);
        }
        pthread_mutex_unlock(&b->mtx);
        return; // ultimul thread trece direct
    }

    pthread_mutex_unlock(&b->mtx);

    // ceilalti asteapta pana cand ultimul thread da drumul barierei
    sem_wait(&b->sem);
}

static void *tfun(void *v)
{
    int tid = *(int *)v;

    usleep((rand() % 200) * 1000);

    printf("%d reached the barrier\n", tid);
    barrier_point(&B);
    printf("%d passed the barrier\n", tid);

    free(v);
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

    for (int i = 0; i < NTHRS; i++) {
        pthread_join(th[i], NULL);
    }

    barrier_destroy(&B);
    return 0;
}

