#ifndef COURSEWORK4_INDEXPROGRAM_H
#define COURSEWORK4_INDEXPROGRAM_H
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace Index {
    using Dictionary = std::unordered_map<std::string, std::unordered_set<std::string>>;
    Dictionary& GetDictionary();
    void Add(const std::string& word, const std::string& file);
    void IndexFilesInRange(const std::vector<std::string>& files);
    std::unordered_set<std::string> FindFilesForWord(const std::string& word);
    std::vector<std::string> FindFilesContainingAllWords(const std::vector<std::string>& files,const std::vector<std::string>& wordsToFind);
}

namespace Indexer {
    std::vector<std::string>
    RunIndexer(const std::string& rootDirectory, const std::vector<std::string>& wordsToFind, int numThreads = 16);
}

namespace Manager {
    std::vector<std::string> GetAllFiles(const std::string &root, int numThreads = 16);
}
#endif //COURSEWORK4_INDEXPROGRAM_H
