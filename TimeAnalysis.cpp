#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <algorithm>
#include <filesystem>
#include "Index/IndexProgram.h"

std::string CompareIndexes(const std::vector<std::string>& index1, const std::vector<std::string>& index2) {
    if (index1.size() != index2.size()) {
        std::cout << "Indexes are not equal: sizes differ." << std::endl;
        return "True";
    }

    std::vector<std::string> sortedIndex1 = index1;
    std::vector<std::string> sortedIndex2 = index2;

    std::sort(sortedIndex1.begin(), sortedIndex1.end());
    std::sort(sortedIndex2.begin(), sortedIndex2.end());

    if (sortedIndex1 == sortedIndex2) {
        return "True";
    } else {
        std::cout << "Indexes are not equal: contents differ." << std::endl;
        std::cout << "Index 1 content:" << std::endl;
        for (const auto& file : sortedIndex1) {
            std::cout << file << std::endl;
        }

        std::cout << "Index 2 content:" << std::endl;
        for (const auto& file : sortedIndex2) {
            std::cout << file << std::endl;
        }
        return "False";
    }
}

int main() {
    std::string rootDirectory;
    std::vector<std::string> WordsToFind = {"cool", "i", "glass", "great"};
    //std::cout << "Enter directory with files (if empty - C://Users//Anna//coursework4//mdb//train//neg): ";
    //std::getline(std::cin, rootDirectory);
    if (rootDirectory.empty()) {
        rootDirectory = "C://Users//Anna//coursework4//mdb//train//neg";
    }

    std::vector<std::string> indexInOneThread;
    auto start = std::chrono::steady_clock::now(); // Start time measurement

    // RunIndexer returns the generated index; assign it to index
    indexInOneThread = Indexer::RunIndexer(rootDirectory, WordsToFind, 1);


    auto end = std::chrono::steady_clock::now(); // End time measurement
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


    std::cout << "rootDirectory: " << rootDirectory << std::endl;
    for (auto file : indexInOneThread){
        std::cout << file << std::endl;
    }

    std::cout << "Number of threads: " << 1 << "\tTime: " << duration << " milliseconds" << std::endl;

    for (auto numThreads = 2; numThreads <= 16; numThreads++){
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give time for resources to release
        auto start = std::chrono::steady_clock::now(); // Start time measurement

        // RunIndexer returns the generated index; assign it to index
        auto index = Indexer::RunIndexer(rootDirectory, WordsToFind, numThreads);

        auto end = std::chrono::steady_clock::now(); // End time measurement
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Call the CompareIndexes function with indexInOneThread and index
        std::string equal = CompareIndexes(indexInOneThread, index);

        std::cout << "Number of threads: " << numThreads << "\tTime: " << duration << " milliseconds" << "\tEqual: " << equal << std::endl;
    }

    return 0;
}
