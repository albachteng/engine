#include <gtest/gtest.h>
#include "../include/FileLoader.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

class FileLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        test_dir = "test_files";
        std::filesystem::create_directory(test_dir);
        
        // Create test files
        createTestFile("test.txt", "Hello, World!\nSecond line.");
        createTestFile("empty.txt", "");
        createTestFile("binary.bin", std::string("\x00\x01\x02\xFF\x7F", 5));
        createTestFile("large.txt", std::string(10000, 'A')); // 10KB file
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(test_dir);
    }

    void createTestFile(const std::string& filename, const std::string& content) {
        std::string filepath = test_dir + "/" + filename;
        std::ofstream file(filepath, std::ios::binary);
        file.write(content.data(), content.size());
        file.close();
    }

    std::string getTestPath(const std::string& filename) {
        return test_dir + "/" + filename;
    }

    std::string test_dir;
};

// ============================================================================
// Safe Methods Tests (Recommended)
// ============================================================================

TEST_F(FileLoaderTest, LoadFileAsString_ValidFile) {
    std::string content = FileLoader::loadFileAsString(getTestPath("test.txt"));
    EXPECT_EQ(content, "Hello, World!\nSecond line.");
}

TEST_F(FileLoaderTest, LoadFileAsString_EmptyFile) {
    std::string content = FileLoader::loadFileAsString(getTestPath("empty.txt"));
    EXPECT_EQ(content, "");
}

TEST_F(FileLoaderTest, LoadFileAsString_NonexistentFile) {
    EXPECT_THROW(
        FileLoader::loadFileAsString(getTestPath("nonexistent.txt")),
        std::runtime_error
    );
}

TEST_F(FileLoaderTest, LoadFileAsString_LargeFile) {
    std::string content = FileLoader::loadFileAsString(getTestPath("large.txt"));
    EXPECT_EQ(content.size(), 10000);
    EXPECT_EQ(content[0], 'A');
    EXPECT_EQ(content[9999], 'A');
}

TEST_F(FileLoaderTest, LoadFileAsBinary_ValidFile) {
    std::vector<char> content = FileLoader::loadFileAsBinary(getTestPath("test.txt"));
    std::string str(content.begin(), content.end());
    EXPECT_EQ(str, "Hello, World!\nSecond line.");
}

TEST_F(FileLoaderTest, LoadFileAsBinary_BinaryData) {
    std::vector<char> content = FileLoader::loadFileAsBinary(getTestPath("binary.bin"));
    EXPECT_EQ(content.size(), 5);
    EXPECT_EQ(static_cast<unsigned char>(content[0]), 0x00);
    EXPECT_EQ(static_cast<unsigned char>(content[1]), 0x01);
    EXPECT_EQ(static_cast<unsigned char>(content[2]), 0x02);
    EXPECT_EQ(static_cast<unsigned char>(content[3]), 0xFF);
    EXPECT_EQ(static_cast<unsigned char>(content[4]), 0x7F);
}

TEST_F(FileLoaderTest, LoadFileAsBinary_EmptyFile) {
    std::vector<char> content = FileLoader::loadFileAsBinary(getTestPath("empty.txt"));
    EXPECT_EQ(content.size(), 0);
}

TEST_F(FileLoaderTest, LoadFileAsBinary_NonexistentFile) {
    EXPECT_THROW(
        FileLoader::loadFileAsBinary(getTestPath("nonexistent.txt")),
        std::runtime_error
    );
}

TEST_F(FileLoaderTest, FileExists_ExistingFile) {
    EXPECT_TRUE(FileLoader::fileExists(getTestPath("test.txt")));
    EXPECT_TRUE(FileLoader::fileExists(getTestPath("empty.txt")));
    EXPECT_TRUE(FileLoader::fileExists(getTestPath("binary.bin")));
}

TEST_F(FileLoaderTest, FileExists_NonexistentFile) {
    EXPECT_FALSE(FileLoader::fileExists(getTestPath("nonexistent.txt")));
    EXPECT_FALSE(FileLoader::fileExists("completely/invalid/path.txt"));
}

// ============================================================================
// Advanced Methods Tests
// ============================================================================

TEST_F(FileLoaderTest, LoadFileAsUniqueCharPtr_ValidFile) {
    auto content = FileLoader::loadFileAsUniqueCharPtr(getTestPath("test.txt"));
    ASSERT_NE(content, nullptr);
    std::string str(content.get());
    EXPECT_EQ(str, "Hello, World!\nSecond line.");
    // Automatic cleanup when content goes out of scope
}

TEST_F(FileLoaderTest, LoadFileAsUniqueCharPtr_EmptyFile) {
    auto content = FileLoader::loadFileAsUniqueCharPtr(getTestPath("empty.txt"));
    ASSERT_NE(content, nullptr);
    EXPECT_EQ(content.get()[0], '\0'); // Should be null-terminated
}

TEST_F(FileLoaderTest, LoadFileAsUniqueCharPtr_NonexistentFile) {
    EXPECT_THROW(
        FileLoader::loadFileAsUniqueCharPtr(getTestPath("nonexistent.txt")),
        std::runtime_error
    );
}

TEST_F(FileLoaderTest, LoadFileAsCharPtr_ValidFile_ManualCleanup) {
    char* content = nullptr;
    try {
        content = FileLoader::loadFileAsCharPtr(getTestPath("test.txt"));
        ASSERT_NE(content, nullptr);
        std::string str(content);
        EXPECT_EQ(str, "Hello, World!\nSecond line.");
        
        FileLoader::freeCharPtr(content);
        content = nullptr; // Prevent double free in catch block
    } catch (...) {
        if (content) FileLoader::freeCharPtr(content);
        throw;
    }
}

TEST_F(FileLoaderTest, LoadFileAsCharPtr_NonexistentFile) {
    EXPECT_THROW(
        FileLoader::loadFileAsCharPtr(getTestPath("nonexistent.txt")),
        std::runtime_error
    );
}

TEST_F(FileLoaderTest, FreeCharPtr_NullptrSafe) {
    // Should not crash or throw
    FileLoader::freeCharPtr(nullptr);
    
    char* ptr = nullptr;
    FileLoader::freeCharPtr(ptr);
}

TEST_F(FileLoaderTest, FreeCharPtr_ValidPointer) {
    char* content = FileLoader::loadFileAsCharPtr(getTestPath("test.txt"));
    ASSERT_NE(content, nullptr);
    
    // Should not crash or leak
    FileLoader::freeCharPtr(content);
}

// ============================================================================
// Comparison Tests - Ensure different methods return same data
// ============================================================================

TEST_F(FileLoaderTest, AllMethods_ReturnSameContent) {
    // Load same file with different methods
    std::string stringContent = FileLoader::loadFileAsString(getTestPath("test.txt"));
    std::vector<char> binaryContent = FileLoader::loadFileAsBinary(getTestPath("test.txt"));
    auto uniquePtrContent = FileLoader::loadFileAsUniqueCharPtr(getTestPath("test.txt"));
    
    char* rawPtrContent = nullptr;
    try {
        rawPtrContent = FileLoader::loadFileAsCharPtr(getTestPath("test.txt"));
        
        // All should contain the same data
        std::string fromBinary(binaryContent.begin(), binaryContent.end());
        std::string fromUniquePtr(uniquePtrContent.get());
        std::string fromRawPtr(rawPtrContent);
        
        EXPECT_EQ(stringContent, fromBinary);
        EXPECT_EQ(stringContent, fromUniquePtr);
        EXPECT_EQ(stringContent, fromRawPtr);
        
        FileLoader::freeCharPtr(rawPtrContent);
    } catch (...) {
        if (rawPtrContent) FileLoader::freeCharPtr(rawPtrContent);
        throw;
    }
}

// ============================================================================
// Performance and Edge Case Tests
// ============================================================================

TEST_F(FileLoaderTest, LargeFile_PerformanceTest) {
    // This tests that large files can be loaded efficiently
    auto start = std::chrono::high_resolution_clock::now();
    std::string content = FileLoader::loadFileAsString(getTestPath("large.txt"));
    auto end = std::chrono::high_resolution_clock::now();
    
    EXPECT_EQ(content.size(), 10000);
    
    // Should complete in reasonable time (less than 100ms for 10KB)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 100);
}

TEST_F(FileLoaderTest, BinaryData_NullBytesHandled) {
    std::vector<char> content = FileLoader::loadFileAsBinary(getTestPath("binary.bin"));
    
    // Should preserve null bytes in binary data
    EXPECT_EQ(content.size(), 5);
    EXPECT_EQ(content[0], '\0'); // Null byte should be preserved
}

// ============================================================================
// Safety Tests
// ============================================================================

TEST_F(FileLoaderTest, ExceptionSafety_StringMethod) {
    // Test that string method is exception safe
    EXPECT_NO_THROW(FileLoader::loadFileAsString(getTestPath("test.txt")));
    
    // Should throw for invalid files but not leak memory
    EXPECT_THROW(
        FileLoader::loadFileAsString("invalid/path/file.txt"),
        std::runtime_error
    );
}

TEST_F(FileLoaderTest, ExceptionSafety_BinaryMethod) {
    // Test that binary method is exception safe
    EXPECT_NO_THROW(FileLoader::loadFileAsBinary(getTestPath("test.txt")));
    
    // Should throw for invalid files but not leak memory
    EXPECT_THROW(
        FileLoader::loadFileAsBinary("invalid/path/file.txt"),
        std::runtime_error
    );
}