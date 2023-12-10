#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <thread>
#include <atomic>

namespace Manager {
    std::mutex fileMutex;

    std::vector<std::string> GetAllFiles(const std::string& root, int numThreads = 16) {
        std::vector<std::string> files;

        std::vector<std::vector<std::string>> fileChunks(numThreads);

        int index = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            fileChunks[index % numThreads].push_back(entry.path().string());
            ++index;
        }

        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([&, i]() {
                for (const auto& file : fileChunks[i]) {
                    std::lock_guard<std::mutex> lock(fileMutex);
                    files.push_back(file);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        return files;
    }
}

namespace Index {
    using Dictionary = std::unordered_map<std::string, std::unordered_set<std::string>>;

    Dictionary& GetDictionary() {
        static Dictionary dictionary;
        return dictionary;
    }

    void Add(const std::string& word, const std::string& file) {
        std::string lowercaseWord = word;
        std::transform(lowercaseWord.begin(), lowercaseWord.end(), lowercaseWord.begin(), ::tolower);

        auto& dictionary = GetDictionary();
        dictionary[lowercaseWord].insert(file);
    }


    std::unordered_set<std::string> FindFilesForWord(const std::string& word) {
        std::unordered_set<std::string> emptySet;

        auto& dictionary = GetDictionary();
        auto it = dictionary.find(word);
        if (it != dictionary.end()) {
            return it->second;
        } else {
            return emptySet;
        }
    }
    std::mutex mutex;

    void IndexFilesInRange(const std::vector<std::string>& files) {
        for (const auto& file : files) {
            std::ifstream ifs(file);

            if (!ifs.is_open()) {
                //std::cerr << "Error opening file: " << file << std::endl;
                continue;
            }

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
                    std::lock_guard<std::mutex> lock(mutex);
                    Index::Add(word, file);
                }
            }
        }
    }

    std::vector<std::string> FindFilesContainingAllWords(const std::vector<std::string>& files,
                                                         const std::vector<std::string>& wordsToFind) {
        std::vector<std::string> filesContainingAllWords;

        for (const auto& file : files) {
            bool foundAllWords = true;

            for (const auto& word : wordsToFind) {
                auto wordFiles = Index::FindFilesForWord(word);
                if (wordFiles.find(file) == wordFiles.end()) {
                    foundAllWords = false;
                    break;
                }
            }
            if (foundAllWords) {
                filesContainingAllWords.push_back(file);
            }
        }
        return filesContainingAllWords;
    }
}

namespace Indexer {

    std::vector<std::string>
    RunIndexer(const std::string& rootDirectory,  const std::vector<std::string>& wordsToFind, int numThreads = 16) {

        std::vector<std::string> files = Manager::GetAllFiles(rootDirectory, numThreads);

        std::vector<std::vector<std::string>> fileChunks(numThreads);
        for (size_t i = 0; i < files.size(); ++i) {
            fileChunks[i % numThreads].push_back(files[i]);
        }

        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([&, i]() {
                Index::IndexFilesInRange(fileChunks[i]);
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::unordered_map<std::string, std::unordered_set<std::string>> wordFilesMap;
        for (const auto& word : wordsToFind) {
            auto wordFiles = Index::FindFilesForWord(word);
            wordFilesMap[word] = wordFiles;
        }
        std::vector<std::string> filesContainingAllWords = Index::FindFilesContainingAllWords(files, wordsToFind);

        return filesContainingAllWords;
    }
}