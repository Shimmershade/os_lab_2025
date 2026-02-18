#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

#define SADDR struct sockaddr

int main(int argc, char **argv) {
  int sockfd, n;
  int serv_port = 20001;
  int bufsize = 1024;
  char *server_ip = NULL;
  int opt;
  
  while ((opt = getopt(argc, argv, "p:b:")) != -1) {
    switch (opt) {
      case 'p':
        serv_port = atoi(optarg);
        break;
      case 'b':
        bufsize = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Usage: %s [-p port] [-b bufsize] <IPaddress of server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  
  if (optind >= argc) {
    printf("usage: %s [-p port] [-b bufsize] <IPaddress of server>\n", argv[0]);
    exit(1);
  }
  
  server_ip = argv[optind];
  
  char sendline[bufsize], recvline[bufsize + 1];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(serv_port);

  if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) < 0) {
    perror("inet_pton problem");
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  printf("Enter string (buffer size = %d, port = %d)\n", bufsize, serv_port);
  write(1, "Enter string\n", 13);

  while ((n = read(0, sendline, bufsize)) > 0) {
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, sizeof(struct sockaddr_in)) == -1) {
      perror("sendto problem");
      exit(1);
    }

    if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1) {
      perror("recvfrom problem");
      exit(1);
    }

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  close(sockfd);
}