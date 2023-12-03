#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace FileManager {
    // find all the files in the dir
    std::vector<std::string> GetAllFiles(const std::string& root) {
        std::vector<std::string> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            files.push_back(entry.path().string());
        }
        return files;
    }
}

namespace Index {
    using Dictionary = std::unordered_map<std::string, std::unordered_set<std::string>>;

    auto &GetDictionaryUnsafe() {
        static Dictionary dictionary;
        return dictionary;
    }

    //add word into dictionary
    void Add(const std::string& word, const std::string& file) {
        std::string lowercaseWord = word;
        std::transform(lowercaseWord.begin(), lowercaseWord.end(), lowercaseWord.begin(), ::tolower);

        auto& dictionary = GetDictionaryUnsafe();
        dictionary[lowercaseWord].insert(file);
    }

    //make sure word from file is lowercase and cleaned
    void IndexFiles(const std::vector<std::string>& files) {
        for (const auto& file : files) {
            std::ifstream ifs(file);
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string text = buffer.str();
            ifs.close();

            std::vector<std::string> words;
            std::stringstream ss(text);
            std::string word;

            while (ss >> word) {
                word.erase(std::remove_if(word.begin(), word.end(), [](char c) { return !std::isalnum(c); }), word.end());
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                if (!word.empty()) {
                    Add(word, file);
                }
            }
        }
    }

    //search for word in dictionary
    std::unordered_set<std::string> FindFilesForWord(const std::string& word) {
        std::unordered_set<std::string> emptySet;

        auto& dictionary = GetDictionaryUnsafe();
        auto it = dictionary.find(word);
        if (it != dictionary.end()) {
            return it->second;
        } else {
            return emptySet;
        }
    }
}

namespace Indexer {
    void RunIndexer(const std::string& rootDirectory, int startFile, int endFile, const std::vector<std::string>& wordsToFind) {
        //find all files in dir
        std::vector<std::string> files = FileManager::GetAllFiles(rootDirectory);

        //choose only the files needed
        if (startFile >= 0 && endFile < files.size()) {
            files.erase(files.begin() + endFile + 1, files.end());
            files.erase(files.begin(), files.begin() + startFile);
        }

        //add words from the files to dictionary
        Index::IndexFiles(files);

        for (const auto& word : wordsToFind) {
            //find if searched word is in the dictionary
            auto wordFiles = Index::FindFilesForWord(word);
            if (!wordFiles.empty()) {
                std::cout << "Word '" << word << "' found in files:\n";
                for (const auto& file : wordFiles) {
                    std::cout << file << std::endl;
                }
                std::cout << "\n";
            } else{
                std::cout << "Word '" << word << "' not found in files:\n";
                std::cout << "\n";
            }
        }
    }
}

int main() {
    int StartOfFile = 0;
    int EndOfFile = 10;
    Index::Dictionary index;
    std::vector<std::string> WordsToFind = {"film", "something", "i", "great", "banana", "are"};
    std::string rootDirectory = "C://Users//Anna//coursework4//mdb//train//neg";
    Indexer::RunIndexer(rootDirectory, StartOfFile, EndOfFile, WordsToFind);
    return 0;
}
