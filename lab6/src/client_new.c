#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

struct Server {
  char ip[255];
  int port;
};

struct ClientArgs {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void* ThreadClient(void* args) {
  struct ClientArgs* cargs = (struct ClientArgs*)args;
  
  struct hostent *hostname = gethostbyname(cargs->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", cargs->server.ip);
    cargs->result = 0;
    return NULL;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(cargs->server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    cargs->result = 0;
    return NULL;
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    close(sck);
    cargs->result = 0;
    return NULL;
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &cargs->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &cargs->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &cargs->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    close(sck);
    cargs->result = 0;
    return NULL;
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Recieve failed\n");
    close(sck);
    cargs->result = 0;
    return NULL;
  }

  memcpy(&cargs->result, response, sizeof(uint64_t));
  close(sck);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers_file[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers_file, optarg, strlen(optarg));
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
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  FILE* fp = fopen(servers_file, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open servers file: %s\n", servers_file);
    return 1;
  }

  struct Server servers[10];
  int servers_num = 0;
  char line[255];
  
  while (fgets(line, sizeof(line), fp) != NULL && servers_num < 10) {
    char* colon = strchr(line, ':');
    if (colon != NULL) {
      *colon = '\0';
      strcpy(servers[servers_num].ip, line);
      servers[servers_num].port = atoi(colon + 1);
      servers_num++;
    }
  }
  fclose(fp);

  if (servers_num == 0) {
    fprintf(stderr, "No valid servers found in file\n");
    return 1;
  }

  pthread_t threads[servers_num];
  struct ClientArgs args[servers_num];
  
  uint64_t step = k / servers_num;
  uint64_t remainder = k % servers_num;
  uint64_t begin = 1;
  
  for (int i = 0; i < servers_num; i++) {
    args[i].server = servers[i];
    args[i].mod = mod;
    args[i].begin = begin;
    
    args[i].end = begin + step - 1;
    if (i < remainder) {
      args[i].end++;
    }
    
    if (args[i].end > k) {
      args[i].end = k;
    }
    
    printf("Server %d: %s:%d will compute %llu to %llu\n", 
           i, servers[i].ip, servers[i].port, args[i].begin, args[i].end);
    
    begin = args[i].end + 1;
    
    if (pthread_create(&threads[i], NULL, ThreadClient, (void*)&args[i])) {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }
  }

  uint64_t total = 1;
  for (int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    printf("Результат с сервера %d: %llu\n", i, args[i].result);
    total *= args[i].result;
  }

  printf("Final answer: %llu\n", total);
  return 0;
}