#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8081

int main() {
  int sockfd;
  char input_char;
  struct sockaddr_in server_addr;

  // 1. Create UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 2. Configure server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);

  // Set server IP to localhost
  if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // 3. Get input from user
  printf("Enter a single uppercase character: ");
  scanf(" %c", &input_char);

  // Basic validation
  if (input_char < 'A' || input_char > 'Z') {
    printf("Error: Only uppercase letters are allowed.\n");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  char buffer[2] = {input_char, '\0'};

  // 4. Send the character to the server
  if (sendto(sockfd, buffer, strlen(buffer), 0,
             (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Sendto failed");
  } else {
    printf("Input from client1: %c sent to server.\n", buffer[0]);
  }

  // 5. Close socket and terminate
  close(sockfd);
  return 0;
}
