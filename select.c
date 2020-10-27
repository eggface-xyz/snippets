#include <vector>
#include <map>
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

using std::vector;
using std::map;

struct client {
		int fd;
		struct sockaddr_in addr;
};

map<int, client> clients;

#define LOCAL_HOST  "127.0.0.1"
#define BUF_SIZE    1024

int startserver(int port)
{
		int max_fd = 0;
    int res;
		int nfds;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) {
        printf("socket() error: %s\n", strerror(errno));
        return errno;
    }

		int on = 1;
		res = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
		if(res < 0) {
				printf("setsockopt() error: %s\n", strerror(errno));
				goto fail;
		}

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    inet_aton(LOCAL_HOST, &addr.sin_addr);
    res = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    if(res < 0) {
        printf("bind() error: %s\n", strerror(errno));
        goto fail;
    }

    res = listen(listen_fd, 5); //backlog
    if(res < 0) {
        printf("listen() error: %s\n", strerror(errno));
        goto fail;
    }

    fd_set readfds;
    FD_SET(listen_fd, &readfds);
		max_fd = max_fd < listen_fd ? listen_fd : max_fd;
    nfds = max_fd + 1;
		printf("server started...\n");
		printf("FDSETSIZE = %d\n", FD_SETSIZE);
    while(true) {
		printf("selecting...\n");
        int cnt = select(nfds, &readfds, NULL, NULL, NULL);
        if(cnt < 0) {
            printf("select error: %s\n", strerror(errno));
            goto fail;
        }
				fd_set alerts = readfds;
				for(int i = 0; i < nfds; i++) {
				    if(FD_ISSET(i, &alerts)) {
                printf("alert from fd %d\n", i);
						    if(i == listen_fd) {
                    struct sockaddr_in client_addr;
                    socklen_t len;
                    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);
                    if(client_fd < 0) {
                        printf("accept error: %s\n", strerror(errno));
                        goto fail;
                    }
                    FD_SET(client_fd, &readfds);
										max_fd = max_fd < client_fd ? client_fd : max_fd;
                    nfds = max_fd + 1;
										clients[i] = client{i, client_addr};
										printf("incoming connection request from %s, current connections: %d\n", 
																		inet_ntoa(client_addr.sin_addr), clients.size());
								} else {
                    printf("process data request\n");
                    uint8_t buffer[BUF_SIZE];
										int n;
										n = read(i, buffer, BUF_SIZE);
						        if(n < 0) {
										    printf("read error\n");
										} else {
										    if(!n) {
														clients.erase(i);
												    printf("disconnection from %s, current connections: %d\n", 
																						inet_ntoa(clients[i].addr.sin_addr), clients.size());
														FD_CLR(i, &readfds);
														close(i);
												} else {
														for(int k = 0; k < n; k++)
														    printf("0x%x, ", buffer[k]);
														puts("-------\n");
												}
										}
								}
						}
				}
    }

    return 0;

fail:
    close(listen_fd);
    return errno;
}

int main(int argc, char* argv[])
{
		if(argc == 1)
				return -1;
    int port = atoi(argv[1]);
    int res = startserver(port);
    if(res < 0)
        return 1;
}
