#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define LOCAL_HOST  "127.0.0.1"
#define BUF_SIZE    1024

int startserver(int port)
{
    int res;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) {
        printf("socket() error: %s\n", strerror(errno));
        return errno;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    inet_aton(LOCAL_HOST, &addr.sin_addr);
    res = bind(listen_fd, &addr, sizeof(addr));
    if(res < 0) {
        printf("bind() error: %s\n", strerror(errno));
        goto fail;
    }

    res = listen(listen_fd, 5);
    if(res < 0) {
        printf("listen() error: %s\n", strerror(errno));
        goto fail;
    }

    fd_set readfds;
    FD_SET(listen_fd, &readfds);
    int nfds = listen_fd + 1;
    while(true) {
        int cnt = select(nfds, &readfds, NULL, NULL, NULL);
        if(cnt < 0) {
            printf("accept error: %s\n", strerror(errno));
            goto fail;
        }
        if(FD_ISSET(listen_fd, readfds)) {
            printf("process connection request\n");
            
            struct sockaddr_in client_addr;
            socklen_t len;
            int client_fd = accept(listen_fd, &client_addr, &len);
            if(client_fd < 0) {
                printf("accept error: %s\n", strerror(errno));
                goto fail;
            }
            FD_SET(client_fd, &readfds);
            nfds = client_fd + 1;
        } else {
            printf("process data request\n");
            uint8_t buffer[BUF_SIZE];
        }
    }

    return 0;

fail:
    close(fd);
    return errno;
}

int main(int argc, char* argv[])
{
    int port = atoi(argv[1]);
    int res = startserver(port);
    if(res < 0)
        return 1;
}
