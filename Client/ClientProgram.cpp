#include <iostream>
#include <asio.hpp>

int main() {
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket socket(io_context);

        socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5001));
        std::cout << "Connected to server!" << std::endl;

        // Send a message to the server
        std::string message = "Hello from client!\n";
        asio::write(socket, asio::buffer(message));

        // Receive and print the response from the server
        asio::streambuf receive_buffer;
        asio::read_until(socket, receive_buffer, '\n');
        std::istream input_stream(&receive_buffer);
        std::string response;
        std::getline(input_stream, response);
        std::cout << "Received response from server: " << response << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
