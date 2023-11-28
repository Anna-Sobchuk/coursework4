#include <iostream>
#include <asio.hpp>
#include <regex>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> GetWords() {
    std::cout << "Please enter the words you want to find: ";
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

int main() {
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket socket(io_context);

        socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5001));
        std::cout << "Connected to server!" << std::endl;

        //Read the words
        std::vector<std::string> words = GetWords();
        for (const auto& word : words) {
            std::cout << " " << word;
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
