#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <future>
#include <mutex>
#include <filesystem>
#include <regex>
#include <string>

namespace Index {
    using Dictionary = std::unordered_map<std::string, std::unordered_set<std::string>>;

    auto& GetDictionaryUnsafe() {
        static Dictionary dictionary;
        return dictionary;
    }

    void Add(std::string word, const std::string& file) {
        for (auto& ch : word) {
            ch = std::tolower(static_cast<unsigned char>(ch));
        }

        auto& dictionary = GetDictionaryUnsafe();
        dictionary[word].insert(file);
    }
}

namespace Indexer {
    void RunIndexer(const std::string& rootDirectory, int numberOfThreads, Index::Dictionary& index, int startFile, int endFile) {
        std::vector<std::string> files;

        // Create file names from startFile to endFile
        for (int i = startFile; i <= endFile; ++i) {
            files.push_back(rootDirectory + "/file" + std::to_string(i) + ".txt");
        }

        // Print the files being processed
        std::cout << "Processing files:" << std::endl;
        for (const auto& file : files) {
            std::cout << file << std::endl;
        }

        // Placeholder: Print words and files, replace with actual indexing logic
        std::cout << "Running indexer..." << std::endl;
        for (const auto& file : files) {
            std::ifstream ifs(file);
            std::string word;
            while (ifs >> word) {
                Index::Add(word, file);
                // Print the word and file being indexed
                std::cout << "Indexed word: " << word << " from file: " << file << std::endl;
            }
            ifs.close();
        }
        std::cout << "Indexing completed." << std::endl;
    }
}

int main() {
    int startFile = 11000;
    int endFile = 11010;

    Index::Dictionary index;
    Indexer::RunIndexer("aclImdb/train/neg", 4, index, startFile, endFile);

    // Print the contents of the index
    std::cout << "Index contents:" << std::endl;
    for (const auto& [word, files] : index) {
        std::cout << "Word: " << word << " Files: ";
        for (const auto& file : files) {
            std::cout << file << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
