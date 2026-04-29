#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sockfd;
  char buffer[BUFFER_SIZE];
  char response[BUFFER_SIZE];
  struct sockaddr_in servaddr, cliaddr;
  socklen_t len;

  // 1. Create UDP datagram socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // 2. Configure server address to bind
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // 3. Bind socket to the designated port
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Voting Server listening on port %d...\n", PORT);

  len = sizeof(cliaddr);

  // 4. Receive a single vote request from a client
  int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                   (struct sockaddr *)&cliaddr, &len);
  if (n < 0) {
    perror("Recvfrom failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  buffer[n] = '\0'; // Null-terminate the received string

  // Remove any trailing newline character sent by the client
  buffer[strcspn(buffer, "\n")] = 0;

  printf("Received vote attempt: '%s'\n", buffer);

  // 5. Validate the vote (Candidates 1, 2, or 3)
  // truncation warnings
  if (strcmp(buffer, "1") == 0 || strcmp(buffer, "2") == 0 ||
      strcmp(buffer, "3") == 0) {
    snprintf(response, sizeof(response), "Vote recorded for Candidate %c",
             buffer[0]);
  } else {
    snprintf(response, sizeof(response), "Invalid vote");
  }

  // 6. Send the confirmation/error message back to the client
  if (sendto(sockfd, response, strlen(response), 0,
             (const struct sockaddr *)&cliaddr, len) < 0) {
    perror("Sendto failed");
  } else {
    printf("Sent reply to client. Terminating server as requested...\n");
  }

  // 7. Close socket and terminate after one request
  close(sockfd);
  return 0;
}
