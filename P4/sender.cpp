#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <chrono>
#include <thread>

#define PORT 8080
#define TIMEOUT 5 // seconds
#define MAX_ATTEMPTS 5

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *messages[] = {"Hello", "This is", "a Stop-and-Wait", "protocol example"};
    int num_messages = sizeof(messages) / sizeof(messages[0]);
    int sequence_number = 0;
    int attempts = 0;
    char ack[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        std::cerr << "Invalid address/ Address not supported \n";
        return -1;
    }

    // Connect to receiver
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed \n";
        return -1;
    }

    std::cout << "Connected to receiver.\n";

    for(int i = 0; i < num_messages; ) {
        // Prepare packet with sequence number and message
        std::string packet = std::to_string(sequence_number) + ":" + messages[i];

        // Send packet
        send(sock , packet.c_str() , packet.length() , 0 );
        std::cout << "Sent packet with sequence number: " << sequence_number << "\n";

        // Set socket timeout
        struct timeval tv;
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        // Wait for ACK
        int valread = read(sock , ack, 1024);

        if(valread > 0) {
            ack[valread] = '\0';
            int ack_num = atoi(ack);
            if(ack_num == sequence_number) {
                std::cout << "Received ACK for sequence number: " << ack_num << "\n";
                sequence_number = 1 - sequence_number; // Toggle sequence number between 0 and 1
                i++;
                attempts = 0; // Reset attempts after successful transmission
            } else {
                std::cout << "Received incorrect ACK. Resending packet.\n";
            }
        } else {
            std::cout << "ACK not received. Timeout occurred. Resending packet.\n";
            attempts++;
            if(attempts >= MAX_ATTEMPTS) {
                std::cerr << "Maximum resend attempts reached. Exiting.\n";
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Optional: Add delay between transmissions
    }

    close(sock);
    std::cout << "Transmission complete. Connection closed.\n";
    return 0;
}
