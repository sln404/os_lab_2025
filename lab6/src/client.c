#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include "mult_modulo.h"

struct Server {
  char ip[255];
  int port;
};

struct ClientThreadArgs {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

void *ServerWorker(void *args) {
  struct ClientThreadArgs *targs = (struct ClientThreadArgs *)args;
  uint64_t *answer = malloc(sizeof(uint64_t));
  *answer = 1;

  struct hostent *hostname = gethostbyname(targs->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed\n");
    return answer;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(targs->server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed to %s:%d\n", targs->server.ip, targs->server.port);
    return answer;
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &targs->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &targs->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &targs->mod, sizeof(uint64_t));

  send(sck, task, sizeof(task), 0);

  recv(sck, answer, sizeof(uint64_t), 0);
  close(sck);

  return (void *)answer;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers_file[255] = {'\0'}; // 255 - max length for a filename

  while (true) {
    static struct option options[] = {
      {"k", required_argument, 0, 0},
      {"mod", required_argument, 0, 0},
      {"servers", required_argument, 0, 0},
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
        k = strtoull(optarg, NULL, 10);
        break;
      case 1:
        mod = strtoull(optarg, NULL, 10);
        break;
      case 2:
        strncpy(servers_file, optarg, 254);
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

  if (k == -1 || mod == -1 || !strlen(servers_file)) {
    fprintf(stdout, "Usage: %s --k \"num\" --mod \"num\" --servers /path/to/file\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(servers_file, "r");
  if (!fp) {
    perror("Cannot open servers file");
    return 1;
  }

  struct Server servers[128];
  int servers_num = 0;
  while (fscanf(fp, "%254[^:]:%d\n", servers[servers_num].ip, &servers[servers_num].port) == 2) {
    servers_num++;
  }
  fclose(fp);

  if (servers_num == 0) {
    fprintf(stderr, "No valid servers found in file.\n");
    return 1;
  }

  pthread_t threads[servers_num];
  struct ClientThreadArgs args[servers_num];

  uint64_t chunk = k / servers_num;
  uint64_t remainder = k % servers_num;
  uint64_t current_start = 1;

  for (int i = 0; i < servers_num; i++) {
    args[i].server = servers[i];
    args[i].begin = current_start;
    
    uint64_t range = chunk + (i < remainder ? 1 : 0);
    args[i].end = current_start + range - 1;
    args[i].mod = mod;
    
    current_start += range;

    pthread_create(&threads[i], NULL, ServerWorker, (void *)&args[i]);
  }

  uint64_t final_answer = 1;
  for (int i = 0; i < servers_num; i++) {
    uint64_t *res;
    pthread_join(threads[i], (void **)&res);
    final_answer = MultModulo(final_answer, *res, mod);
    free(res);
  }

  printf("answer: %lu\n", final_answer);

  return 0;
}
