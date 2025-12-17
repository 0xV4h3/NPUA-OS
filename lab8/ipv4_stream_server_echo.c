#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "ipv4_dg.h"

#define BACKLOG 5
#define MAX_CLIENTS FD_SETSIZE

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int sfd, cfd, i, maxfd, nready, client[MAX_CLIENTS];
    ssize_t numRead;
    char buf[BUF_SIZE];
    fd_set allset, rset;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) errExit("socket");

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT_NUM);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
        errExit("bind");

    if (listen(sfd, BACKLOG) == -1) errExit("listen");

    for (i = 0; i < MAX_CLIENTS; i++)
        client[i] = -1;

    FD_ZERO(&allset);
    FD_SET(sfd, &allset);
    maxfd = sfd;

    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) errExit("select");

        if (FD_ISSET(sfd, &rset)) {
            cfd = accept(sfd, NULL, NULL);
            if (cfd == -1) errExit("accept");
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client[i] < 0) {
                    client[i] = cfd;
                    break;
                }
            }
            if (i == MAX_CLIENTS) {
                fprintf(stderr, "Too many clients\n");
                close(cfd);
            } else {
                FD_SET(cfd, &allset);
                if (cfd > maxfd) maxfd = cfd;
            }
            if (--nready <= 0) continue;
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            int sockfd;
            if ((sockfd = client[i]) < 0) continue;
            if (FD_ISSET(sockfd, &rset)) {
                numRead = read(sockfd, buf, BUF_SIZE);
                if (numRead <= 0) {
                    if (numRead < 0) perror("read");
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    // Echo received message back to the sender
                    if (write(sockfd, buf, numRead) != numRead)
                        perror("echo write error");
                }
                if (--nready <= 0) break;
            }
        }
    }
}