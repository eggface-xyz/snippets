#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

