#include <iostream>
#include <asio.hpp>
#include <regex>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace ClientMessager {
    void Send(asio::ip::tcp::socket &socket, const std::vector<std::string> &words) {
        try {
            // Send the size of the vector
            uint32_t vectorSize = static_cast<uint32_t>(words.size());
            asio::write(socket, asio::buffer(&vectorSize, sizeof(vectorSize)));

            // Send the vector of words
            for (const auto &word: words) {
                uint32_t wordSize = static_cast<uint32_t>(word.size());
                asio::write(socket, asio::buffer(&wordSize, sizeof(wordSize)));
                asio::write(socket, asio::buffer(word.data(), word.size()));
            }
        } catch (const std::exception &e) {
            std::cerr << "Error sending message: " << e.what() << std::endl;
        }
    }

    std::vector<std::string> ReceiveFileList(asio::ip::tcp::socket &socket) {
        // Read the size of the vector
        uint32_t vectorSize;
        asio::read(socket, asio::buffer(&vectorSize, sizeof(vectorSize)));

        std::vector<std::string> fileList;

        // Read the vector of file names
        for (uint32_t i = 0; i < vectorSize; ++i) {
            uint32_t fileEntrySize;
            asio::read(socket, asio::buffer(&fileEntrySize, sizeof(fileEntrySize)));

            std::vector<char> fileBuffer(fileEntrySize);
            asio::read(socket, asio::buffer(fileBuffer));

            std::string file(fileBuffer.begin(), fileBuffer.end());
            fileList.push_back(file);
        }

        return fileList;
    }
}

std::vector<std::string> GetWords() {
    std::cout << "Please enter the words you want to find:" << std::endl;
    std::string input;
    std::getline(std::cin, input);

    std::regex word_regex(R"([^(0-9|a-z|A-Z|')]+)");
    std::sregex_token_iterator iter(input.begin(), input.end(), word_regex, -1);
    std::sregex_token_iterator end;

    std::vector<std::string> words;
    for (; iter != end; ++iter) {
        std::string word = iter->str();
        if (!word.empty()) {
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            words.push_back(word);
        }
    }

    return words;
}

bool WantToContinue() {
    std::cout << "If you want to finish, write to the console 'yes'. If not, press enter." << std::endl;
    std::string input;
    std::getline(std::cin, input);

    return input != "yes";
}

void SendDisconnectMessage(asio::ip::tcp::socket& socket) {
    // Send a special disconnect message
    ClientMessager::Send(socket, { "DISCONNECT" });
}

int main() {
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket socket(io_context);

        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), 5001);
        socket.connect(endpoint);

        std::cout << "Connected to server!" << std::endl;

        do {
            std::vector<std::string> words = GetWords();
            ClientMessager::Send(socket, words);

            std::vector<std::string> fileList = ClientMessager::ReceiveFileList(socket);

            std::cout << "Server response:\n" << std::endl;
            if (fileList.empty()) {
                std::cout << "The specified words were not found in any files." << std::endl;
            } else {
                for (const auto& file : fileList) {
                    std::cout << file << std::endl;
                }
            }
            std::cout << std::endl;
        } while (WantToContinue());

        //send last message about disconnection
        SendDisconnectMessage(socket);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
