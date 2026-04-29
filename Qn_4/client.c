#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE] = {0};
  char request[BUFFER_SIZE] = {0};
  int num1, num2;
  char op;

  // 1. Create TCP stream socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 address from text to binary
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address or Address not supported \n");
    close(sock);
    return -1;
  }

  // 2. Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed. Ensure the server is running.\n");
    close(sock);
    return -1;
  }

  // 3. Read inputs from the user
  printf("Enter first number: ");
  if (scanf("%d", &num1) != 1)
    return -1;

  printf("Enter second number: ");
  if (scanf("%d", &num2) != 1)
    return -1;

  printf("Enter operator: ");
  // The space before %c is crucial to consume the stray newline left by
  // previous scanf
  if (scanf(" %c", &op) != 1)
    return -1;

  // 4. Format inputs into a single string to send
  snprintf(request, sizeof(request), "%d %d %c", num1, num2, op);

  // 5. Send request to server
  send(sock, request, strlen(request), 0);

  // 6. Receive and display the server's response
  int valread = recv(sock, buffer, BUFFER_SIZE, 0);
  if (valread > 0) {
    printf("\n%s\n", buffer);
  } else {
    printf("\nError: Failed to receive data from server.\n");
  }

  // 7. Clean up and close socket
  close(sock);
  return 0;
}
