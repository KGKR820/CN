#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8081
#define CLIENT2_PORT 8082

// Order to run : client2 , server,client1

int main() {
  int sockfd;
  char buffer[2]; // Buffer to hold the single character + null terminator
  struct sockaddr_in server_addr, client1_addr, client2_addr;
  socklen_t addr_len = sizeof(client1_addr);

  // 1. Create UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 2. Configure server address to bind
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(SERVER_PORT);

  // 3. Bind socket to server port
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", SERVER_PORT);

  // 4. Receive data from client 1
  if (recvfrom(sockfd, buffer, sizeof(buffer), 0,
               (struct sockaddr *)&client1_addr, &addr_len) < 0) {
    perror("Recvfrom failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Received '%c' from client1.\n", buffer[0]);

  // 5. Decrement the character
  if (buffer[0] == 'A') {
    buffer[0] = 'Z'; // Handle wrap-around for A -> Z
  } else {
    buffer[0] = buffer[0] - 1;
  }

  printf("Decrementing... new character is '%c'.\n", buffer[0]);

  // 6. Configure Client 2 address
  memset(&client2_addr, 0, sizeof(client2_addr));
  client2_addr.sin_family = AF_INET;
  client2_addr.sin_port = htons(CLIENT2_PORT);
  if (inet_pton(AF_INET, "127.0.0.1", &client2_addr.sin_addr) <= 0) {
    perror("Invalid address for client2");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // 7. Send the updated character to client 2
  if (sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&client2_addr,
             sizeof(client2_addr)) < 0) {
    perror("Sendto failed");
  } else {
    printf("Sent '%c' to client2.\n", buffer[0]);
  }

  // 8. Close socket and terminate
  close(sockfd);
  return 0;
}
