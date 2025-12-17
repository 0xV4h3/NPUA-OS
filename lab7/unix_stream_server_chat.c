#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include "unix_stream.h"

#define BACKLOG 5
#define MAX_CLIENTS FD_SETSIZE

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd, cfd, i, maxfd, nready, client[MAX_CLIENTS];
    ssize_t numRead;
    char buf[BUF_SIZE];
    fd_set allset, rset;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
        perror("remove");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        if (errno == EADDRINUSE) {
            fprintf(stderr, "Address already in use\n");
        } else {
            perror("bind");
        }
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < MAX_CLIENTS; i++) client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(sfd, &allset);
    maxfd = sfd;

    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sfd, &rset)) {
            cfd = accept(sfd, NULL, NULL);
            if (cfd == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client[i] < 0) {
                    client[i] = cfd;
                    break;
                }
            }
            if (i == MAX_CLIENTS) {
                fprintf(stderr, "too many clients\n");
                close(cfd);
            } else {
                FD_SET(cfd, &allset);
                if (cfd > maxfd) maxfd = cfd;
            }
            if (--nready <= 0)
                continue;
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            int sockfd;
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                numRead = read(sockfd, buf, BUF_SIZE);
                if (numRead <= 0) {
                    if (numRead < 0) perror("read");
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    // Broadcast to all clients except the sender
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        int outfd = client[j];
                        if (outfd >= 0 && outfd != sockfd) {
                            if (write(outfd, buf, numRead) != numRead) {
                                perror("broadcast");
                            }
                        }
                    }
                }
                if (--nready <= 0)
                    break;
            }
        }
    }
}