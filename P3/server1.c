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
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = { 0 };
    char *welcome_message = "Welcome to the server!";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Accept an incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Send a welcome message to the client
        send(new_socket, welcome_message, strlen(welcome_message), 0);
        printf("Welcome message sent\n");

        // Handle client communication
        while (1) {
            valread = read(new_socket, buffer, BUFFER_SIZE - 1);
            if (valread > 0) {
                buffer[valread] = '\0'; // Null-terminate the received data
                printf("Received message: %s\n", buffer);

                // If the client sends "exit", close the connection
                if (strcmp(buffer, "exit") == 0) {
                    printf("Client disconnected.\n");
                    break;
                }

                // Echo the message back to the client
                send(new_socket, buffer, strlen(buffer), 0);
                printf("Message echoed back to client\n");
            } else {
                printf("Client disconnected or error occurred.\n");
                break;
            }
        }

        // Close the socket after the client disconnects
        close(new_socket);
    }

    return 0;
}