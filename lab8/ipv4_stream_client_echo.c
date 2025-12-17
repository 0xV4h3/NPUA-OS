#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "ipv4_dg.h"

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int sfd;
    ssize_t numRead, numWritten, numRecvd;
    char buf[BUF_SIZE];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        errExit("socket");
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
        errExit("inet_pton failed");
    }

    if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        errExit("connect");
    }

    // Fork: child reads from server and prints to stdout, parent sends stdin to server
    pid_t pid = fork();
    if (pid == -1) {
        errExit("fork");
    }

    if (pid == 0) {
        // Child: receives all messages from server
        while((numRecvd = read(sfd, buf, BUF_SIZE)) > 0) {
            if (write(STDOUT_FILENO, buf, numRecvd) != numRecvd) {
                fprintf(stderr, "stdout write error\n");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    }

    // Parent: sends stdin to server
    while((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        numWritten = write(sfd, buf, numRead);
        if (numWritten != numRead) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }
    }

    close(sfd);
    exit(EXIT_SUCCESS);
}