#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char filename[BUFFER_SIZE];
  char buffer[BUFFER_SIZE];

  // Create stream socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  // Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed. Is the server running?\n");
    return -1;
  }

  printf("Connected to server.\n");
  printf("Enter the filename to request: ");

  if (fgets(filename, sizeof(filename), stdin) == NULL) {
    printf("Error reading input.\n");
    close(sock);
    return -1;
  }

  filename[strcspn(filename, "\n")] = 0; // Remove trailing newline

  if (strlen(filename) == 0) {
    printf("Filename cannot be empty.\n");
    close(sock);
    return -1;
  }

  // 1. Send the requested filename to the server
  write(sock, filename, strlen(filename));

  // 2. Read the 1-byte status header from the server
  char status;
  if (read(sock, &status, 1) <= 0) {
    printf("Error receiving response from server.\n");
    close(sock);
    return -1;
  }

  // 3. Process response based on status
  if (status == '0') {
    // File not found error
    int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
      buffer[bytes_read] = '\0';
      printf("\n>>> Server Message: %s\n", buffer);
    }
  } else if (status == '1') {
    // File exists, prepare to receive and save
    char new_filename[BUFFER_SIZE + 10];
    snprintf(new_filename, sizeof(new_filename), "received_%s", filename);

    // Low-level file creation (0644 gives rw-r--r-- permissions)
    int fd = creat(new_filename, 0644);
    if (fd < 0) {
      perror("Error creating local file");
      close(sock);
      return -1;
    }

    printf("Receiving file data...\n");
    int bytes_read;
    int total_bytes = 0;

    // Loop to read from socket and write to file
    while ((bytes_read = read(sock, buffer, sizeof(buffer))) > 0) {
      if (write(fd, buffer, bytes_read) < 0) {
        perror("Error writing to local file");
        break;
      }
      total_bytes += bytes_read;
    }

    printf("Transfer successful! Saved %d bytes to '%s'.\n", total_bytes,
           new_filename);
    close(fd); // Close the local file
  } else {
    printf("Unknown status code received from server.\n");
  }

  // Close socket
  close(sock);
  return 0;
}
