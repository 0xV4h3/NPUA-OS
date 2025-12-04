#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static void *threadFunc(void *arg)
{
    char *s = (char *)arg;
    printf("[detached thread] started: %s\n", s ? s : "(null)");
    sleep(1);
    printf("[detached thread] finishing\n");
    return NULL;
}

int main(void)
{
    pthread_t t;
    int s;

    s = pthread_create(&t, NULL, threadFunc, "Hello from detached");
    if (s != 0) {
        fprintf(stderr, "pthread_create error\n");
        return EXIT_FAILURE;
    }

    /* Detach the thread so main doesn't join it */
    s = pthread_detach(t);
    if (s != 0) {
        fprintf(stderr, "pthread_detach error\n");
        return EXIT_FAILURE;
    }

    printf("[main] created and detached thread, exiting main (sleep shortly to observe detached thread)\n");
    sleep(2);
    return EXIT_SUCCESS;
}
