#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};
  char response[BUFFER_SIZE] = {0};

  // 1. Create TCP stream socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Attach socket to the port to avoid "Address already in use"
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // 2. Bind the socket
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 3. Listen for connections
  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Calculator Server listening on port %d...\n", PORT);

  // 4. Accept and process client requests sequentially
  while (1) {
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0) {
      perror("Accept failed");
      continue;
    }

    memset(buffer, 0, BUFFER_SIZE);
    memset(response, 0, BUFFER_SIZE);

    // 5. Receive the formatted data from the client
    int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (valread > 0) {
      int num1, num2;
      char op;

      // Parse the received string
      if (sscanf(buffer, "%d %d %c", &num1, &num2, &op) == 3) {
        printf("Received request: %d %c %d\n", num1, op, num2);

        // 6. Perform the arithmetic operation
        switch (op) {
        case '+':
          snprintf(response, sizeof(response), "Result = %d", num1 + num2);
          break;
        case '-':
          snprintf(response, sizeof(response), "Result = %d", num1 - num2);
          break;
        case '*':
          snprintf(response, sizeof(response), "Result = %d", num1 * num2);
          break;
        case '/':
          if (num2 == 0) {
            snprintf(response, sizeof(response),
                     "Error: Division by zero is not allowed.");
          } else {
            // Using float format just in case, or sticking strictly to integer
            // math
            snprintf(response, sizeof(response), "Result = %d", num1 / num2);
          }
          break;
        default:
          snprintf(response, sizeof(response),
                   "Error: Invalid operator '%c'. Supported: +, -, *, /", op);
          break;
        }
      } else {
        snprintf(response, sizeof(response), "Error: Malformed input data.");
      }

      // 7. Send the response back to the client
      send(client_socket, response, strlen(response), 0);
      printf("Sent to client: %s\n\n", response);
    }

    // 8. Close client connection
    close(client_socket);
  }

  close(server_fd);
  return 0;
}
