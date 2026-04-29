#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CLIENT2_PORT 8082

int main() {
  int sockfd;
  char buffer[2] = {0};
  struct sockaddr_in client2_addr, server_addr;
  socklen_t addr_len = sizeof(server_addr);

  // 1. Create UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 2. Configure Client 2 address to bind
  memset(&client2_addr, 0, sizeof(client2_addr));
  client2_addr.sin_family = AF_INET;
  client2_addr.sin_addr.s_addr = INADDR_ANY;
  client2_addr.sin_port = htons(CLIENT2_PORT);

  // 3. Bind socket to listen for the server's message
  if (bind(sockfd, (const struct sockaddr *)&client2_addr,
           sizeof(client2_addr)) < 0) {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Client 2 waiting for data on port %d...\n", CLIENT2_PORT);

  // 4. Receive data from the server
  if (recvfrom(sockfd, buffer, sizeof(buffer), 0,
               (struct sockaddr *)&server_addr, &addr_len) < 0) {
    perror("Recvfrom failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // 5. Display the received character
  printf("\nOutput at client2: %c\n", buffer[0]);

  // 6. Close socket and terminate
  close(sockfd);
  return 0;
}
