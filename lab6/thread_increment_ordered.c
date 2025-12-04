#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static int glob = 0;

static void *threadFunc(void *arg)
{
    int loops = *(int *)arg;
    int loc, j;

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int loops, s;
    loops = (argc > 1) ? atoi(argv[1]) : 1000000;

    int *loopsP1 = malloc(sizeof(int));
    int *loopsP2 = malloc(sizeof(int));
    if (!loopsP1 || !loopsP2) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    *loopsP1 = loops;
    *loopsP2 = loops;

    /* Create first thread and wait for it to finish before creating second */
    s = pthread_create(&t1, NULL, threadFunc, loopsP1);
    if (s != 0) {
        fprintf(stderr, "pthread_create error\n");
        return EXIT_FAILURE;
    }

    s = pthread_join(t1, NULL);
    if (s != 0) {
        fprintf(stderr, "pthread_join error\n");
        return EXIT_FAILURE;
    }

    /* Now start second thread */
    s = pthread_create(&t2, NULL, threadFunc, loopsP2);
    if (s != 0) {
        fprintf(stderr, "pthread_create error\n");
        return EXIT_FAILURE;
    }

    s = pthread_join(t2, NULL);
    if (s != 0) {
        fprintf(stderr, "pthread_join error\n");
        return EXIT_FAILURE;
    }

    printf("glob = %d\n", glob);
    free(loopsP1);
    free(loopsP2);
    return EXIT_SUCCESS;
}
