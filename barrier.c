// barrier_pthread.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NTHRS 5

static pthread_barrier_t bar;

static void *tfun(void *v)
{
    int tid = *(int *)v;
    free(v);

    usleep((rand() % 200) * 1000);

    printf("%d reached the barrier\n", tid);
    fflush(stdout);

    int rc = pthread_barrier_wait(&bar);
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
        fprintf(stderr, "pthread_barrier_wait failed: %d\n", rc);
        return NULL;
    }

    printf("%d passed the barrier\n", tid);
    fflush(stdout);

    return NULL;
}

int main(void)
{
    srand(12345);

    if (pthread_barrier_init(&bar, NULL, NTHRS) != 0) {
        perror("pthread_barrier_init");
        return 1;
    }

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

    pthread_barrier_destroy(&bar);
    return 0;
}

