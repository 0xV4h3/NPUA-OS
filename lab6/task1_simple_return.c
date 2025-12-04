#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static void *threadFunc(void *arg)
{
    char *s = (char *)arg;
    if (s == NULL)
        return NULL;

    size_t *len = malloc(sizeof(size_t));
    if (len == NULL) {
        perror("malloc");
        return NULL;
    }

    *len = strlen(s);
    printf("[thread] received: '%s'\n", s);
    return (void *)len;
}

int main(int argc, char *argv[])
{
    pthread_t t;
    void *res;
    int s;
    const char *msg = (argc > 1) ? argv[1] : "Hello world";

    s = pthread_create(&t, NULL, threadFunc, (void *)msg);
    if (s != 0) {
        fprintf(stderr, "pthread_create error\n");
        return EXIT_FAILURE;
    }

    s = pthread_join(t, &res);
    if (s != 0) {
        fprintf(stderr, "pthread_join error\n");
        return EXIT_FAILURE;
    }

    if (res) {
        printf("[main] returned length = %zu\n", *(size_t *)res);
        free(res);
    } else {
        printf("[main] thread returned NULL\n");
    }

    return EXIT_SUCCESS;
}
