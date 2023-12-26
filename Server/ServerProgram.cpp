#include <iostream>
#include <asio.hpp>
#include <thread>
#include <vector>
#include <unordered_set>
#include "Index/IndexProgram.h"
#include <csignal>
#include <filesystem>
#include <queue>

std::mutex mutex_;
std::condition_variable condition_;
std::queue<asio::ip::tcp::socket> socketQueue;

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

void sendMessage(asio::ip::tcp::socket& socket, const std::vector<std::string>& files) {
    // Send the size of the vector
    uint32_t vectorSize = static_cast<uint32_t>(files.size());
    asio::write(socket, asio::buffer(&vectorSize, sizeof(vectorSize)));

    // Send the vector of files
    for (const auto& file : files) {
        std::string fileEntry = file;
        uint32_t fileEntrySize = static_cast<uint32_t>(fileEntry.size());
        asio::write(socket, asio::buffer(&fileEntrySize, sizeof(fileEntrySize)));
        asio::write(socket, asio::buffer(fileEntry.data(), fileEntry.size()));
    }
}

void handleClient(asio::ip::tcp::socket socketToProcess) {
    try {
        std::cout << "Client thread started" << std::endl;

        while (true) {
            std::vector<std::string> words = receiveWords(socketToProcess);

            std::string rootDirectory = "C://Users//Anna//coursework4//mdb//test"; // put this for the speed, but can be changed

            // Check for disconnect message
            if (words.size() == 1 && words[0] == "DISCONNECT") {
                std::cout << "Client disconnected." << std::endl;
                break;
            }

            std::cout << "Indexing started" << std::endl;
            std::vector<std::string> files = Indexer::RunIndexer(rootDirectory, words);
            std::cout << "Indexing finished" << std::endl;

            // Send a response back to the client
            sendMessage(socketToProcess, files);
        }
    } catch (const asio::system_error &e) {
        if (e.code() != asio::error::eof) {
            std::cerr << "Error handling client: " << e.what() << std::endl;
        }
    }
}

int main() {
    asio::io_service ioService;
    asio::ip::tcp::acceptor acceptor(ioService, asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5001));

    std::cout << "Server started at port 5001" << std::endl;
    std::vector<std::thread> threads;
    const int THREAD_POOL_SIZE = 4;

    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        threads.emplace_back([&ioService]() {
            while (true) {
                asio::ip::tcp::socket socket(ioService);
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [&]() { return !socketQueue.empty(); });

                    socket = std::move(socketQueue.front());
                    socketQueue.pop();
                }

                handleClient(std::move(socket));
            }
        });
    }

    while (true) {
        asio::ip::tcp::socket socket(ioService);
        acceptor.listen();
        acceptor.accept(socket);
        std::cout << "New client was connected" << std::endl;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            socketQueue.push(std::move(socket));
        }

        condition_.notify_one(); // Notify a waiting thread to process the client
    }

    for (auto& thread : threads) {
        thread.join();
    }

    //Delete saved index file before shutting down the server
    std::filesystem::remove("Saved.bin");
    std::cout << "Index file deleted. Server shutting down." << std::endl;

    return 0;
}