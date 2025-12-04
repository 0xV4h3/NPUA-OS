#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static int data[10];

static void *fill_even(void *arg)
{
    (void)arg;
    for (int i = 0; i < 10; i += 2) {
        data[i] = i + 1; /* fill with 1..10 values */
    }
    return NULL;
}

int main(void)
{
    pthread_t t_even;
    int s;

    s = pthread_create(&t_even, NULL, fill_even, NULL);
    if (s != 0) { fprintf(stderr, "pthread_create error\n"); return EXIT_FAILURE; }

    /* main fills odd indices */
    for (int i = 1; i < 10; i += 2) {
        data[i] = i + 1;
    }

    s = pthread_join(t_even, NULL);
    if (s != 0) { fprintf(stderr, "pthread_join error\n"); return EXIT_FAILURE; }

    printf("data = ");
    for (int i = 0; i < 10; ++i) {
        printf("%d ", data[i]);
    }
    printf("\n");

    return EXIT_SUCCESS;
}
