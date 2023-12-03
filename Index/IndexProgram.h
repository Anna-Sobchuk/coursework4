#ifndef COURSEWORK4_INDEXPROGRAM_H
#define COURSEWORK4_INDEXPROGRAM_H
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace Index {
    using Dictionary = std::unordered_map<std::string, std::unordered_set<std::string>>;
    auto &GetDictionaryUnsafe();
    void Add(const std::string& word, const std::string& file);
    void IndexFiles(const std::vector<std::string>& files);
    std::unordered_set<std::string> FindFilesForWord(const std::string& word);
}

namespace Indexer {
    std::unordered_map<std::string, std::unordered_set<std::string>>
    RunIndexer(const std::string& rootDirectory, int startFile, int endFile, const std::vector<std::string>& wordsToFind);
}

namespace FileManager {
    std::vector<std::string> GetAllFiles(const std::string &root);
}
#endif //COURSEWORK4_INDEXPROGRAM_H
