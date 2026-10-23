#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int port = -1;
  int bufsize = -1;
  int backlog = -1;

  while (true) {
    static struct option options[] = {
      {"port", required_argument, 0, 0},
      {"bufsize", required_argument, 0, 0},
      {"backlog", required_argument, 0, 0},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        break;
      case 1:
        bufsize = atoi(optarg);
        break;
      case 2:
        backlog = atoi(optarg);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port <= 0 || bufsize <= 0 || backlog <= 0) {
    fprintf(stderr, "Usage: %s --port \"num\" --bufsize \"num\" --backlog \"num\"\n", argv[0]);
    return 1;
  }

  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  int nread;
  char *buf = malloc(bufsize);
  if (!buf) {
    perror("malloc");
    exit(1);
  }

  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    free(buf);
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(lfd, (struct sockaddr *)&servaddr, kSize) < 0) {
    perror("bind");
    free(buf);
    exit(1);
  }

  if (listen(lfd, backlog) < 0) {
    perror("listen");
    free(buf);
    exit(1);
  }

  while (true) {
    unsigned int clilen = kSize;

    if ((cfd = accept(lfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      free(buf);
      exit(1);
    }
    printf("connection established\n");

    while ((nread = read(cfd, buf, bufsize)) > 0) {
      write(1, buf, nread);
    }

    if (nread == -1) {
      perror("read");
      free(buf);
      exit(1);
    }
    close(cfd);
  }

  free(buf);
  close(lfd);
  return 0;
}
