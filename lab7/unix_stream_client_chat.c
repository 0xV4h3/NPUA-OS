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

    // Create a child process to read messages from the server and print them to the stdout
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork error\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: reads incoming messages from the server and prints to stdout
        while ((numRecvd = read(sfd, buf, BUF_SIZE)) > 0) {
            if (write(STDOUT_FILENO, buf, numRecvd) != numRecvd) {
                fprintf(stderr, "stdout write error\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }

    // Parent process: reads from stdin and sends to the server
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        numWritten = write(sfd, buf, numRead);
        if (numWritten != numRead) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        fprintf(stderr, "read error\n");
        exit(EXIT_FAILURE);
    }

    // Close socket and exit client
    close(sfd);
    exit(EXIT_SUCCESS);
}