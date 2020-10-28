#include <map>
#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

using std::map;

struct client {
  int fd;
  struct sockaddr_in addr;
};

map<int, client> clients;

#define BUF_SIZE 1024

int setup_listening_socket(int port) {
  int res;
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    printf("socket() error: %s\n", strerror(errno));
    return errno;
  }

  int on = 1;
  res =
      setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  if (res < 0) {
    printf("setsockopt() error: %s\n", strerror(errno));
    goto fail;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = INADDR_ANY;
  res = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (res < 0) {
    printf("bind() error: %s\n", strerror(errno));
    goto fail;
  }

  res = listen(listen_fd, 5); // backlog
  if (res < 0) {
    printf("listen() error: %s\n", strerror(errno));
    goto fail;
  }

  return listen_fd;

fail:
  close(listen_fd);
  return -1;
}

int startserver(int port) {
  int res;
  int nfds;

  int listen_fd = setup_listening_socket(port);
  if (listen_fd < 0) {
    printf("setup listening socket error\n");
    return -1;
  }

  printf("server started...\n");
  while (true) {
    fd_set readfds;
    FD_ZERO(
        &readfds); // it's very important to zero fdset before every iteration
    int maxfd = listen_fd;
    FD_SET(listen_fd, &readfds);
    for (auto e : clients) {
      maxfd = std::max(maxfd, e.first);
      FD_SET(e.first, &readfds);
    }
    // printf("selecting... maxfd + 1 = %d\n", maxfd + 1);
    int cnt = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (cnt < 0) {
      printf("select error: %s\n", strerror(errno));
      goto fail;
    }
    if (FD_ISSET(listen_fd, &readfds)) {
      struct sockaddr_in client_addr;
      socklen_t len = sizeof(client_addr);
      int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
      // accept(listen_fd, NULL, NULL);
      if (client_fd < 0) {
        printf("accept error: %s\n", strerror(errno));
        goto fail;
      }
      clients[client_fd] = client{client_fd, client_addr};
      printf("incoming connection request from %s, current connections: %d\n",
             inet_ntoa(client_addr.sin_addr), (int)clients.size());
    }
    for (int i = 0; i <= maxfd; i++) {
      if (i == listen_fd)
        continue;
      if (FD_ISSET(i, &readfds)) {
        printf("process data request\n");
        uint8_t buffer[BUF_SIZE];
        int n;
        n = read(i, buffer, BUF_SIZE);
        if (n < 0)
          printf("read error\n");
        else {
          if (!n) {
            close(i);
            printf("disconnection from %s on fd %d, current connections: %d\n",
                   inet_ntoa(clients[i].addr.sin_addr), i, (int)clients.size());
            clients.erase(i);
          } else {
            printf("recv data from %s\n", inet_ntoa(clients[i].addr.sin_addr));
            for (int k = 0; k < n; k++)
              printf("0x%x, ", buffer[k]);
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

int main(int argc, char *argv[]) {
  if (argc == 1)
    return -1;
  int port = atoi(argv[1]);
  int res = startserver(port);
  if (res < 0)
    return 1;
}
