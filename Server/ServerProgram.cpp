#include <iostream>
#include <asio.hpp>
#include <thread>
#include <vector>
#include <unordered_set>
#include "Index/IndexProgram.h"
std::atomic<bool> shouldStop(false); // Global variable to signal server shutdown


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

void sendMessage(asio::ip::tcp::socket& socket, const std::unordered_map<std::string, std::unordered_set<std::string>>& dictionary) {
    // Send the size of the dictionary
    uint32_t dictionarySize = static_cast<uint32_t>(dictionary.size());
    asio::write(socket, asio::buffer(&dictionarySize, sizeof(dictionarySize)));

    // Send the dictionary of word:files
    for (const auto& entry : dictionary) {
        std::string word = entry.first;
        uint32_t wordSize = static_cast<uint32_t>(word.size());
        asio::write(socket, asio::buffer(&wordSize, sizeof(wordSize)));
        asio::write(socket, asio::buffer(word.data(), word.size()));

        uint32_t fileListSize = static_cast<uint32_t>(entry.second.size());
        asio::write(socket, asio::buffer(&fileListSize, sizeof(fileListSize)));

        for (const auto& file : entry.second) {
            std::string fileEntry = file;
            uint32_t fileEntrySize = static_cast<uint32_t>(fileEntry.size());
            asio::write(socket, asio::buffer(&fileEntrySize, sizeof(fileEntrySize)));
            asio::write(socket, asio::buffer(fileEntry.data(), fileEntry.size()));
        }
    }
}


void handleClient(asio::ip::tcp::socket socket) {
    try {
        std::cout << "Client thread started!" << std::endl;

        while (true) {
            std::vector<std::string> words = receiveWords(socket);

            int StartOfFile = 0;
            int EndOfFile = 10;
            std::string rootDirectory = "C://Users//Anna//coursework4//mdb//train//neg";
            // Run indexing on the words and get the updated dictionary
            Index::Dictionary dictionary = Indexer::RunIndexer(rootDirectory, StartOfFile, EndOfFile, words);

            // Check for disconnect message
            if (words.size() == 1 && words[0] == "DISCONNECT") {
                std::cout << "Client disconnected." << std::endl;
                break;
            }

            // Send a response back to the client
            sendMessage(socket, dictionary);
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

    const int MAX_CLIENTS = 2; // Define maximum number of clients
    int clientCount = 0;

    while (!shouldStop) {
        asio::ip::tcp::socket socket(ioService);
        acceptor.listen();
        acceptor.accept(socket);
        std::cout << "New client was connected" << std::endl;

        // Start a new thread for each client
        threads.emplace_back(handleClient, std::move(socket));

        ++clientCount;
        if (clientCount >= MAX_CLIENTS) {
            shouldStop = true; // Reached maximum clients, set the termination condition
        }
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}