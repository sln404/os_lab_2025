#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int port = -1;
  int bufsize = -1;

  while (true) {
    static struct option options[] = {
      {"port", required_argument, 0, 0},
      {"bufsize", required_argument, 0, 0},
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

  if (port <= 0 || bufsize <= 0) {
    fprintf(stderr, "Usage: %s --port \"num\" --bufsize \"num\"\n", argv[0]);
    return 1;
  }

  const size_t kSize = sizeof(struct sockaddr_in);

  int sockfd, n;
  char *mesg = malloc(bufsize + 1);
  if (!mesg) {
    perror("malloc");
    exit(1);
  }
  char ipadr[16];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    free(mesg);
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *)&servaddr, kSize) < 0) {
    perror("bind problem");
    free(mesg);
    exit(1);
  }
  printf("server starts...\n");

  while (true) {
    unsigned int len = kSize;

    if ((n = recvfrom(sockfd, mesg, bufsize, 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
      perror("recvfrom");
      free(mesg);
      exit(1);
    }
    mesg[n] = 0;

    printf("request %s      from %s : %d\n", mesg,
           inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
           ntohs(cliaddr.sin_port));

    if (sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len) < 0) {
      perror("sendto");
      free(mesg);
      exit(1);
    }
  }

  free(mesg);
  close(sockfd);
  return 0;
}
