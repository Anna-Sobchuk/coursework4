#define CATCH_CONFIG_MAIN

#include <filesystem>
#include "catch.hpp"
#include "Index/IndexProgram.h"

TEST_CASE("GetAllFiles returns files in directory") {

    std::string testDirectory = "C://Users//Anna//coursework4//mdb";

    SECTION("Check if GetAllFiles returns files") {
        std::vector<std::string> files = Manager::GetAllFiles(testDirectory);

        REQUIRE(files.size() > 0);
    }
}

TEST_CASE("Add words to index") {

    std::string tempFile = "temp.txt";
    std::string anotherFile = "anotherFile.txt";

    Index::Add("apple", tempFile);
    Index::Add("banana", tempFile);
    Index::Add("apple", anotherFile);

    SECTION("Check if words are added correctly") {
        Index::Dictionary& wordFiles = Index::GetDictionary();

        REQUIRE(wordFiles.size() == 2);
        REQUIRE(wordFiles["apple"].size() == 2);
        REQUIRE(wordFiles["banana"].size() == 1);
    }

}
TEST_CASE("FindFilesForWord returns correct files") {

    std::string tempFile = "temp.txt";
    std::string anotherFile = "anotherFile.txt";

    Index::Add("lemon", tempFile);
    Index::Add("kiwi", tempFile);
    Index::Add("lemon", anotherFile);

    SECTION("Check files for 'lemon'") {

        auto appleFiles = Index::FindFilesForWord("lemon");

        REQUIRE(appleFiles.size() == 2);
        REQUIRE(appleFiles.find(tempFile) != appleFiles.end());
        REQUIRE(appleFiles.find(anotherFile) != appleFiles.end());
    }

    SECTION("Check files for 'kiwi'") {

        auto bananaFiles = Index::FindFilesForWord("kiwi");


        REQUIRE(bananaFiles.size() == 1);
        REQUIRE(bananaFiles.find(tempFile) != bananaFiles.end());
    }

    SECTION("Check files for non-existing word") {

        auto nonExistingFiles = Index::FindFilesForWord("nonexistent");

        REQUIRE(nonExistingFiles.empty());
    }
}
TEST_CASE("IndexFilesInRange indexes files correctly") {

    std::string tempFile = "temp.txt";

    std::string tempFileContent = "this is an apple";

    std::ofstream tempStream(tempFile);
    tempStream << tempFileContent;
    tempStream.close();

    std::vector<std::string> files= {tempFile};
    Index::IndexFilesInRange(files);

    SECTION("Check indexing for 'apple'") {
        auto appleFiles = Index::FindFilesForWord("apple");

        // Check if 'apple' is indexed correctly
        REQUIRE(appleFiles.find(tempFile) != appleFiles.end());
    }

    SECTION("Check indexing for non-existing word") {
        auto nonExistingFiles = Index::FindFilesForWord("nonexistent");

        REQUIRE(nonExistingFiles.empty());
    }
}
TEST_CASE("RunIndexer returns files for specified words") {

    std::string tempFile = "temp.txt";
    std::string anotherFile = "anotherFile.txt";

    std::string tempFileContent = "this is an apple";
    std::string anotherFileContent = "apple and banana";

    std::ofstream tempStream(tempFile);
    tempStream << tempFileContent;
    tempStream.close();

    std::ofstream anotherStream(anotherFile);
    anotherStream << anotherFileContent;
    anotherStream.close();

    std::vector<std::string> wordsToFind = {"apple", "banana"};
    std::vector<std::string> foundFiles = Indexer::RunIndexer("C://Users//Anna//coursework4//Tests", wordsToFind);

    SECTION("Check if files contain specified words") {

        for (const auto& file : foundFiles) {
            auto wordFiles = Index::FindFilesForWord("apple");
            wordFiles.insert(Index::FindFilesForWord("banana").begin(), Index::FindFilesForWord("banana").end());
            if (wordFiles.find(file) == wordFiles.end()) {
                FAIL("Specified words not found in the file");
            }
        }
    }
}

TEST_CASE("FindFilesContainingAllWords handles files with no matching words") {

    Index::GetDictionary().clear();

    Index::Add("apple", "file1.txt");
    Index::Add("banana", "file2.txt");

    std::vector<std::string> files = {"file3.txt", "file4.txt"};
    std::vector<std::string> wordsToFind = {"orange", "grape"};

    std::vector<std::string> resultFiles = Index::FindFilesContainingAllWords(files, wordsToFind);

    REQUIRE(resultFiles.empty());
}
TEST_CASE("FindFilesContainingAllWords handles empty input") {

    Index::GetDictionary().clear();

    std::vector<std::string> files;
    std::vector<std::string> wordsToFind;

    std::vector<std::string> resultFiles = Index::FindFilesContainingAllWords(files, wordsToFind);

    REQUIRE(resultFiles.empty());
}
TEST_CASE("RunIndexer handles empty input") {

    std::vector<std::string> wordsToFind;
    std::vector<std::string> foundFiles = Indexer::RunIndexer("C://Users//Anna//coursework4//Tests//emptydir", wordsToFind);

    REQUIRE(foundFiles.empty());
}
TEST_CASE("IndexFilesInRange handles unreal files") {

    std::vector<std::string> unrealFiles = {
            "nonexistent_file.txt",
            "fake_file.docx"
    };

    Index::IndexFilesInRange(unrealFiles);

    for (const auto& file : unrealFiles) {
        auto filesForWord = Index::FindFilesForWord(file);
        REQUIRE(filesForWord.empty());
    }
}

TEST_CASE("SerializeIndex saves index to file") {
    Index::Add("test", "file1.txt");
    Index::Add("sample", "file2.txt");

    Index::SerializeIndex("test_index.bin");

    REQUIRE(std::filesystem::exists("test_index.bin"));
}

TEST_CASE("DeserializeIndex loads index from file") {
    Index::Add("test", "file1.txt");
    Index::Add("sample", "file2.txt");

    Index::SerializeIndex("test_index_to_load.bin");

    Index::GetDictionary().clear();

    Index::DeserializeIndex("test_index_to_load.bin");

    REQUIRE(Index::FindFilesForWord("test").count("file1.txt") == 1);
    REQUIRE(Index::FindFilesForWord("sample").count("file2.txt") == 1);
}

TEST_CASE("File gets deleted") {
    std::ofstream testFile("test_file.txt");
    testFile.close();

    REQUIRE(std::filesystem::exists("test_file.txt"));

    std::filesystem::remove("test_file.txt");

    REQUIRE_FALSE(std::filesystem::exists("test_file.txt"));
}