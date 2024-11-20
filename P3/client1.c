#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h> // For exit()

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const* argv[])
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = { 0 };
    char message[BUFFER_SIZE];

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server. Type 'exit' to quit.\n");

    while (1) {
        // Prompt user for input
        printf("Enter message: ");
        fgets(message, BUFFER_SIZE, stdin);

        // Remove newline character if present
        message[strcspn(message, "\n")] = 0;

        // Check for exit condition
        if (strcmp(message, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Send the message to the server
        send(client_fd, message, strlen(message), 0);
        printf("Message sent\n");

        // Read response from server
        valread = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (valread > 0) {
            buffer[valread] = '\0'; // Null-terminate the received data
            printf("Server response: %s\n", buffer);
        } else {
            printf("Server disconnected or error occurred.\n");
            break;
        }
    }

    // Close the connected socket
    close(client_fd);
    return 0;
}