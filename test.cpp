#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

constexpr int PORT = 25565;
constexpr size_t BUFFER_SIZE = 4096;

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 8) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client), &len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "Client connected!\n";
        char buffer[BUFFER_SIZE];

        // simple bounded read loop
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            std::cout << "Received " << bytes << " bytes\n";
            // echo or process data here
            send(client_fd, buffer, bytes, 0);
        }

        close(client_fd);
        std::cout << "Client disconnected.\n";
    }

    close(server_fd);
    return 0;
}