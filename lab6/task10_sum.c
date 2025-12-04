#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static int data[10];

struct sum_args {
    int start;
    int len;
};

static void *partial_sum(void *arg)
{
    struct sum_args *a = (struct sum_args *)arg;
    long *res = malloc(sizeof(long));
    if (!res) { perror("malloc"); return NULL; }
    *res = 0;
    for (int i = a->start; i < a->start + a->len; ++i) *res += data[i];
    return res;
}

int main(void)
{
    /* initialize data 1..10 */
    for (int i = 0; i < 10; ++i) data[i] = i + 1;

    pthread_t t1, t2;
    struct sum_args a1 = { .start = 0, .len = 5 };
    struct sum_args a2 = { .start = 5, .len = 5 };
    void *r1, *r2;
    int s;

    s = pthread_create(&t1, NULL, partial_sum, &a1);
    if (s != 0) { fprintf(stderr, "pthread_create error\n"); return EXIT_FAILURE; }

    s = pthread_create(&t2, NULL, partial_sum, &a2);
    if (s != 0) { fprintf(stderr, "pthread_create error\n"); return EXIT_FAILURE; }

    s = pthread_join(t1, &r1);
    if (s != 0) { fprintf(stderr, "pthread_join error\n"); return EXIT_FAILURE; }

    s = pthread_join(t2, &r2);
    if (s != 0) { fprintf(stderr, "pthread_join error\n"); return EXIT_FAILURE; }

    long total = 0;
    if (r1) { total += *(long *)r1; free(r1); }
    if (r2) { total += *(long *)r2; free(r2); }

    printf("sum first 5 + last 5 = %ld\n", total);
    return EXIT_SUCCESS;
}
