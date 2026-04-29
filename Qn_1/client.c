#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define SIZE 1024

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[SIZE] = {0};
  char input[SIZE];

  // 1. Create stream socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form (localhost here)
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    close(sock);
    return -1;
  }

  // 2. Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed. Is the server running?\n");
    close(sock);
    return -1;
  }

  printf("Successfully connected to the server.\n");

  // 3. Read numeric address from user
  printf("Enter the numeric address to search: ");
  if (fgets(input, sizeof(input), stdin) == NULL) {
    printf("Error reading input.\n");
    close(sock);
    return -1;
  }

  // Strip out the newline character from fgets
  input[strcspn(input, "\n")] = 0;

  // Handle empty input error
  if (strlen(input) == 0) {
    printf("Invalid input. Address cannot be empty.\n");
    close(sock);
    return -1;
  }

  // 4. Send address to server
  send(sock, input, strlen(input), 0);
  printf("Request sent. Waiting for response...\n");

  // 5. Receive response from server
  int valread = recv(sock, buffer, SIZE, 0);
  if (valread > 0) {
    printf("\n>>> Server Response: %s <<<\n", buffer);
  } else {
    printf("\nError receiving data from server.\n");
  }

  // 6. Close the socket
  close(sock);
  return 0;
}
