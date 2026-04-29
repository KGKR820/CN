#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define SIZE 1024
#define DB_FILE "database.txt"

int main() {
  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[SIZE] = {0};

  // 1. Create stream socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Attach socket to the port to avoid "Address already in use" errors
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; // Bind to any available local interface
  address.sin_port = htons(PORT);

  // 2. Bind the socket to the network address and port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 3. Listen for incoming connections (queue limit of 3)
  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  // Handle one client request at a time sequentially
  while (1) {
    // 4. Accept an incoming connection
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0) {
      perror("Accept failed");
      continue;
    }

    printf("\n--- New Client Connected ---\n");

    memset(buffer, 0, SIZE);

    // 5. Receive the numeric address from the client
    int valread = recv(client_socket, buffer, SIZE, 0);
    if (valread <= 0) {
      printf("Client disconnected or recv error.\n");
      close(client_socket);
      continue;
    }

    int search_addr = atoi(buffer); // Convert received string to integer
    printf("Received request for address: %d\n", search_addr);

    // 6. Open database and search
    FILE *file = fopen(DB_FILE, "r");
    char response[SIZE] = "Address not found";

    if (file == NULL) {
      perror("Error opening database file");
      strcpy(response, "Server Error: Database file not found");
    } else {
      int db_addr;
      char db_name[256];

      // Read file line by line
      while (fscanf(file, "%d %s", &db_addr, db_name) != EOF) {
        if (db_addr == search_addr) {
          strcpy(response, db_name); // Found the name
          break;
        }
      }
      fclose(file); // Properly close the file
    }

    // 7. Send the response back to the client
    printf("Sending response: %s\n", response);
    send(client_socket, response, strlen(response), 0);

    // 8. Close client socket after fulfilling the request
    close(client_socket);
    printf("Client socket closed. Waiting for next request...\n");
  }

  // Close server socket (unreachable in this infinite loop, but good practice)
  close(server_fd);
  return 0;
}
