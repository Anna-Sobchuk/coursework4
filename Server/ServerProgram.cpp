#include <iostream>
#include <asio.hpp>
#include <thread>
#include <vector>

std::vector<std::string> receiveWords(asio::ip::tcp::socket& socket) {
    // Read the size of the vector
    uint32_t vectorSize;
    asio::read(socket, asio::buffer(&vectorSize, sizeof(vectorSize)));

    // Read the vector of words
    std::vector<std::string> words;
    for (uint32_t i = 0; i < vectorSize; ++i) {
        uint32_t wordSize;
        asio::read(socket, asio::buffer(&wordSize, sizeof(wordSize)));

        std::vector<char> buffer(wordSize);
        asio::read(socket, asio::buffer(buffer));

        std::string word(buffer.begin(), buffer.end());
        words.push_back(word);
    }

    return words;
}

void sendMessage(asio::ip::tcp::socket& socket, const std::string& message) {
    // Send the size of the message
    uint32_t messageSize = static_cast<uint32_t>(message.size());
    asio::write(socket, asio::buffer(&messageSize, sizeof(messageSize)));

    // Send the message
    asio::write(socket, asio::buffer(message.data(), message.size()));
}

void handleClient(asio::ip::tcp::socket socket) {
    try {
        std::cout << "Client thread started!" << std::endl;

        while (true) {
            std::vector<std::string> words = receiveWords(socket);

            // Process the received words as needed
            std::cout << "Received words: ";
            for (const auto& word : words) {
                std::cout << word << " ";
            }
            std::cout << std::endl;

            // Indexing

            // Check for disconnect message
            if (words.size() == 1 && words[0] == "DISCONNECT") {
                std::cout << "Client disconnected." << std::endl;
                break;
            }

            // Send a response back to the client
            std::string responseMessage = "Hey, the words were found!";
            sendMessage(socket, responseMessage);
        }
    } catch (const asio::system_error& e) {
        if (e.code() != asio::error::eof) {
            std::cerr << "Error handling client: " << e.what() << std::endl;
        }
    }
}

int main() {
    asio::io_service ioService;
    asio::ip::tcp::acceptor acceptor(ioService, asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5001));

    std::cout << "Server started at port 5001" << std::endl;
    std::vector<std::thread> threads; // Keep track of the threads

    while (true) {
        asio::ip::tcp::socket socket(ioService);
        acceptor.listen();
        acceptor.accept(socket);
        std::cout << "New client was connected" << std::endl;

        // Start a new thread for each client
        threads.emplace_back(handleClient, std::move(socket));
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
