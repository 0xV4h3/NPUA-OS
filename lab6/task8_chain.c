#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static void *double_thread(void *arg)
{
    int *v = (int *)arg;
    if (!v) return NULL;
    int *res = malloc(sizeof(int));
    if (!res) { perror("malloc"); return NULL; }
    *res = (*v) * 2;
    printf("[t1] doubled %d -> %d\n", *v, *res);
    return res;
}

static void *triple_thread(void *arg)
{
    int *v = (int *)arg;
    if (!v) return NULL;
    int *res = malloc(sizeof(int));
    if (!res) { perror("malloc"); return NULL; }
    *res = (*v) * 3;
    printf("[t2] tripled %d -> %d\n", *v, *res);
    return res;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    pthread_t t1, t2;
    void *res1, *res2;
    int s;

    int *arg1 = malloc(sizeof(int));
    if (!arg1) { perror("malloc"); return EXIT_FAILURE; }
    *arg1 = n;

    s = pthread_create(&t1, NULL, double_thread, arg1);
    if (s != 0) { fprintf(stderr, "pthread_create error\n"); return EXIT_FAILURE; }

    s = pthread_join(t1, &res1);
    if (s != 0) { fprintf(stderr, "pthread_join error\n"); return EXIT_FAILURE; }

    /* pass doubled value to second thread */
    s = pthread_create(&t2, NULL, triple_thread, res1);
    if (s != 0) { fprintf(stderr, "pthread_create error\n"); return EXIT_FAILURE; }

    s = pthread_join(t2, &res2);
    if (s != 0) { fprintf(stderr, "pthread_join error\n"); return EXIT_FAILURE; }

    if (res2) {
        printf("[main] final result = %d\n", *(int *)res2);
        free(res2);
    }
    free(arg1);
    return EXIT_SUCCESS;
}
