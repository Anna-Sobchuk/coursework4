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

        // dividing the files search for threads
        std::vector<std::vector<std::string>> fileChunks(numThreads);

        int index = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            fileChunks[index % numThreads].push_back(entry.path().string());
            ++index;
        }

        // adding all found files to one for further operations
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

    Dictionary& GetDictionary() { // form in which intes exists
        static Dictionary dictionary;
        return dictionary;
    }

    void Add(const std::string& word, const std::string& file) { // adding a new word to index
        std::string lowercaseWord = word;
        std::transform(lowercaseWord.begin(), lowercaseWord.end(), lowercaseWord.begin(), ::tolower);

        auto& dictionary = GetDictionary();
        dictionary[lowercaseWord].insert(file);
    }


    std::unordered_set<std::string> FindFilesForWord(const std::string& word) { //check if the word is in the dictionary
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

    void IndexFilesInRange(const std::vector<std::string>& files) { // making the index itself
        for (const auto& file : files) {
            std::ifstream ifs(file);

            if (!ifs.is_open()) {
                //std::cerr << "Error opening file: " << file << std::endl; gives error during error tests, that's why muted
                continue;
            }

            //getting words for each file
            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string text = buffer.str();
            ifs.close();

            std::vector<std::string> words;
            std::stringstream ss(text);
            std::string word;

            //add each word to the dictionary
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

            //going through files with words and checking which files contain all words
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

    void SerializeIndex(const std::string& filename) { // save index
        std::lock_guard<std::mutex> lock(mutex);
        std::ofstream outputFile(filename, std::ios::binary | std::ios::trunc);
        if (outputFile.is_open()) {
            auto& dictionary = GetDictionary();
            size_t size = dictionary.size();
            outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& pair : dictionary) {
                size_t wordSize = pair.first.size();
                outputFile.write(reinterpret_cast<const char*>(&wordSize), sizeof(wordSize));
                outputFile.write(pair.first.data(), wordSize);
                size_t filesSize = pair.second.size();
                outputFile.write(reinterpret_cast<const char*>(&filesSize), sizeof(filesSize));
                for (const auto& file : pair.second) {
                    size_t fileSize = file.size();
                    outputFile.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
                    outputFile.write(file.data(), fileSize);
                }
            }
            outputFile.close();
        } else {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
        }
    }


    void DeserializeIndex(const std::string& filename) { //open local index
        std::lock_guard<std::mutex> lock(mutex);
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            Dictionary dictionary;
            size_t size;
            file.read(reinterpret_cast<char*>(&size), sizeof(size));
            for (size_t i = 0; i < size; ++i) {
                size_t wordSize;
                file.read(reinterpret_cast<char*>(&wordSize), sizeof(wordSize));
                std::string word(wordSize, '\0');
                file.read(&word[0], wordSize);
                size_t filesSize;
                file.read(reinterpret_cast<char*>(&filesSize), sizeof(filesSize));
                std::unordered_set<std::string> files;
                for (size_t j = 0; j < filesSize; ++j) {
                    size_t fileSize;
                    file.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
                    std::string fileStr(fileSize, '\0');
                    file.read(&fileStr[0], fileSize);
                    files.insert(fileStr);
                }
                dictionary[word] = files;
            }
            file.close();
            GetDictionary() = dictionary;
        } else {
            std::cerr << "Error opening file for reading: " << filename << std::endl;
        }
    }

}

namespace Indexer {

    std::vector<std::string>
    RunIndexer(const std::string& rootDirectory,  const std::vector<std::string>& wordsToFind, int numThreads = 16) {

        std::vector<std::string> files = Manager::GetAllFiles(rootDirectory, numThreads);


        std::string indexFileName = "C://Users//Anna//coursework4//Index//IndSaved.bin";
        if (std::filesystem::exists(indexFileName)) {
            // Deserialize the index if it exists
            Index::DeserializeIndex(indexFileName);
        } else {
            // If the index file doesn't exist, build the index
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
            // Serialize the index to a file
            Index::SerializeIndex(indexFileName);
        }

        //find if words are in the files
        std::unordered_map<std::string, std::unordered_set<std::string>> wordFilesMap;
        for (const auto& word : wordsToFind) {
            auto wordFiles = Index::FindFilesForWord(word);
            wordFilesMap[word] = wordFiles;
        }
        std::vector<std::string> filesContainingAllWords = Index::FindFilesContainingAllWords(files, wordsToFind);

        return filesContainingAllWords;
    }
}