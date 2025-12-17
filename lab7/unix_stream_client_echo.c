#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "unix_stream.h"

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd;
    ssize_t numRead, numWritten, numRecvd;
    char buf[BUF_SIZE];

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sfd == -1) {
        fprintf(stderr, "socket error\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        fprintf(stderr, "connect error\n");
        exit(EXIT_FAILURE);
    }

    /* Copy stdin to socket and read echo back */
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        numWritten = write(sfd, buf, numRead);
        if (numWritten != numRead) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }

        // Read echo back from server
        numRecvd = read(sfd, buf, BUF_SIZE);
        if (numRecvd < 0) {
            fprintf(stderr, "read (echo) error\n");
            exit(EXIT_FAILURE);
        }
        if (numRecvd > 0) {
            if (write(STDOUT_FILENO, buf, numRecvd) != numRecvd) {
                fprintf(stderr, "partial/failed write (stdout)\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (numRead == -1) {
        fprintf(stderr, "read error\n");
        exit(EXIT_FAILURE);
    }

    close(sfd);
    exit(EXIT_SUCCESS);
}