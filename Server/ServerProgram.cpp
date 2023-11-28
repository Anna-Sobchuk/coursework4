#include <iostream>
#include <asio.hpp>
#include <thread>
#include <vector>
#include <chrono>
#include <condition_variable>


std::vector<std::string> ReceiveWords(asio::ip::tcp::socket& socket) {

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

void HandleClient(asio::ip::tcp::socket socket) {
    try {
        std::cout << "Client thread started!" << std::endl;

        while (true) {
            std::vector<std::string> words = ReceiveWords(socket);

            // Process the received words as needed
            std::cout << "Received words: ";
            for (const auto& word : words) {
                std::cout << word << " ";
            }
            std::cout << std::endl;

            // Now, as the words are received, start an index search itself
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
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
        threads.emplace_back(HandleClient, std::move(socket));

    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}