#include <iostream>
#include <experimental/filesystem>
#include <asio.hpp>

namespace fs = std::experimental::filesystem;

class Server {
private:
    asio::io_service ioService;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket socket;

public:
    Server() : acceptor(ioService, asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5001)), socket(ioService) {}

    void Start() {
        acceptor.listen();
        std::cout << "Server started at port 5001" << std::endl;

        while (true) {
            acceptor.accept(socket);
            std::cout << "New client was connected" << std::endl;

            // Start a new thread or use async to handle the client
            HandleClient(std::move(socket));
        }
    }

    void HandleClient(asio::ip::tcp::socket socket) {
        try {
            asio::streambuf buffer;
            asio::read_until(socket, buffer, '\n');
            std::istream input_stream(&buffer);
            std::string message;
            std::getline(input_stream, message);

            // Process the received message
            std::cout << "Received message from client: " << message << std::endl;

            // You can send a response back to the client if needed
            asio::write(socket, asio::buffer("Hello from server!\n"));
        }
        catch (const std::exception& e) {
            std::cerr << "Error handling client: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {

    Server server;
    server.Start();

    return 0;
}
