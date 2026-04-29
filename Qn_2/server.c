#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Signal handler to prevent zombie processes
void handle_sigchld(int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

void handle_client(int client_socket) {
  char filename[BUFFER_SIZE] = {0};
  char buffer[BUFFER_SIZE];
  int bytes_read;

  // 1. Read the requested filename from the client
  if (read(client_socket, filename, sizeof(filename) - 1) <= 0) {
    perror("Error reading from socket");
    close(client_socket);
    return;
  }

  printf("[Child PID: %d] Requested file: %s\n", getpid(), filename);

  // 2. Try to open the requested file (Low-level file I/O)
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    // File does not exist
    printf("[Child PID: %d] File '%s' not found. Sending error.\n", getpid(),
           filename);
    char status = '0'; // '0' indicates error
    write(client_socket, &status, 1);
    char err_msg[] = "Error: File not found on server.\n";
    write(client_socket, err_msg, strlen(err_msg));
  } else {
    // File exists
    printf("[Child PID: %d] File found. Transferring data...\n", getpid());
    char status = '1'; // '1' indicates success/file data follows
    write(client_socket, &status, 1);

    // Read from file and write to socket in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
      if (write(client_socket, buffer, bytes_read) < 0) {
        perror("Error writing to socket");
        break;
      }
    }
    printf("[Child PID: %d] File transfer complete.\n", getpid());
    close(fd); // Close the file
  }

  // 3. Close the client socket
  close(client_socket);
}

int main() {
  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Prevent zombie processes
  signal(SIGCHLD, handle_sigchld);

  // Create stream socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Allow port reuse
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind socket
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  if (listen(server_fd, 5) < 0) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Concurrent Server listening on port %d...\n", PORT);

  while (1) {
    // Accept incoming connection
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0) {
      perror("Accept failed");
      continue;
    }

    printf("\n--- New Client Connected ---\n");

    // Create child process for concurrent handling
    pid_t pid = fork();

    if (pid < 0) {
      perror("Fork failed");
      close(client_socket);
    } else if (pid == 0) {
      // Child process
      close(server_fd); // Child doesn't need the listening socket
      handle_client(client_socket);
      exit(0); // Terminate child after serving the client
    } else {
      // Parent process
      close(client_socket); // Parent doesn't need the connected socket
    }
  }

  close(server_fd);
  return 0;
}
