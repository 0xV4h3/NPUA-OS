#ifndef GET_ELEMENT_H
#define GET_ELEMENT_H

#include <stdio.h>
#include <string.h>

#define ARR_SIZE 5
#define MAX_LENGTH 64

static char *array[ARR_SIZE] = {
    "Element 1",
    "Element 2",
    "Element 3",
    "Element 4",
    "Element 5",
};

static char buf[MAX_LENGTH];
/* If you want thread-local storage, uncomment the line below and adjust usage:
static __thread char buf[MAX_LENGTH]; */

static inline char *getElement(int index)
{
    if (index < 0 || index >= ARR_SIZE) {
        fprintf(stderr, "Unknown element\n");
        return NULL;
    }

    strncpy(buf, array[index], MAX_LENGTH - 1);
    buf[MAX_LENGTH - 1] = '\0';
    return buf;
}

#endif /* GET_ELEMENT_H */
