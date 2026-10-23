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
  char ip[255] = {'\0'};

  while (true) {
    static struct option options[] = {
      {"ip", required_argument, 0, 0},
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
        strncpy(ip, optarg, 254);
        break;
      case 1:
        port = atoi(optarg);
        break;
      case 2:
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

  if (port <= 0 || bufsize <= 0 || !strlen(ip)) {
    fprintf(stderr, "Usage: %s --ip \"ip_addr\" --port \"num\" --bufsize \"num\"\n", argv[0]);
    return 1;
  }

  const size_t kSize = sizeof(struct sockaddr_in);

  int sockfd, n;
  char *sendline = malloc(bufsize);
  char *recvline = malloc(bufsize + 1);
  if (!sendline || !recvline) {
    perror("malloc");
    exit(1);
  }

  struct sockaddr_in servaddr;

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
    perror("inet_pton problem");
    free(sendline);
    free(recvline);
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    free(sendline);
    free(recvline);
    exit(1);
  }

  write(1, "Enter string\n", 13);

  while ((n = read(0, sendline, bufsize)) > 0) {
    if (sendto(sockfd, sendline, n, 0, (struct sockaddr *)&servaddr, kSize) == -1) {
      perror("sendto problem");
      free(sendline);
      free(recvline);
      exit(1);
    }

    int nrecv;
    if ((nrecv = recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL)) == -1) {
      perror("recvfrom problem");
      free(sendline);
      free(recvline);
      exit(1);
    }

    recvline[nrecv] = 0;
    printf("reply from server= %s\n", recvline);
  }

  free(sendline);
  free(recvline);
  close(sockfd);
  return 0;
}
