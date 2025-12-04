#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static void * threadFunc(void *arg)
{
    char *s = (char *) arg;
    if (s == NULL) {
        return NULL;
    }

    printf("%s\n", s);
    size_t length = strlen(s);
    size_t *lengthP = malloc(sizeof(size_t));
    if (lengthP == NULL) {
        perror("malloc");
        return NULL;
    }

    *lengthP = length;
    return (void *) lengthP;
}

int main(int argc, char *argv[])
{
    pthread_t t1;
    void *res;
    int s;
    s = pthread_create(&t1, NULL, threadFunc, "Hello world");

    if (s != 0) {
        fprintf(stderr, "pthread_create error");
        exit(EXIT_FAILURE);
    }

    printf("Message from main()\n");
    s = pthread_join(t1, &res);

    if (s != 0) {
        fprintf(stderr, "pthread_join error");
        exit(EXIT_FAILURE);
    }

    if (res != NULL) {
        printf("Thread returned %zu\n", *(size_t*) res);
        free(res);
    } else {
        fprintf(stderr, "Thread returned NULL\n");
    }
    exit(EXIT_SUCCESS);
}
