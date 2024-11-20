#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    int expected_sequence = 0;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed\n";
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the PORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed\n";
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0) {
        std::cerr << "Bind failed\n";
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Receiver is listening on port " << PORT << "...\n";

    // Accept incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0) {
        std::cerr << "Accept failed\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Connection established with sender.\n";

    while(true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read( new_socket , buffer, 1024);
        if(valread <= 0) {
            std::cout << "No more data received. Closing connection.\n";
            break;
        }
        buffer[valread] = '\0';

        // Parse sequence number and message
        std::string data(buffer);
        size_t delimiter_pos = data.find(":");
        if(delimiter_pos == std::string::npos) {
            std::cerr << "Invalid packet format received.\n";
            continue;
        }

        int seq_num = std::stoi(data.substr(0, delimiter_pos));
        std::string message = data.substr(delimiter_pos + 1);

        if(seq_num == expected_sequence) {
            std::cout << "Received packet with sequence number: " << seq_num << "\n";
            std::cout << "Message: " << message << "\n";
            // Send ACK
            std::string ack = std::to_string(seq_num);
            send(new_socket , ack.c_str() , ack.length() , 0 );
            std::cout << "Sent ACK for sequence number: " << seq_num << "\n";
            expected_sequence = 1 - expected_sequence; // Toggle expected sequence number
        } else {
            std::cout << "Received out-of-order packet. Discarding and resending ACK for sequence number: " << 1 - expected_sequence << "\n";
            // Resend ACK for last received in-order packet
            std::string ack = std::to_string(1 - expected_sequence);
            send(new_socket , ack.c_str() , ack.length() , 0 );
        }
    }

    close(new_socket);
    close(server_fd);
    std::cout << "Connection closed.\n";
    return 0;
}
