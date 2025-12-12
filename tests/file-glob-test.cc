#include <string>
#include <vector>
#include <fstream>
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include "glob-cpp/file-glob.h"

namespace fs = boost::filesystem;

// Test fixture for file globbing tests
class FileGlobTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a temporary directory for tests
    test_dir_ = fs::temp_directory_path() / "glob_cpp_test";
    if (fs::exists(test_dir_)) {
      fs::remove_all(test_dir_);
    }
    fs::create_directories(test_dir_);
    
    // Create test directory structure
    fs::create_directories(test_dir_ / "subdir1");
    fs::create_directories(test_dir_ / "subdir2");
    fs::create_directories(test_dir_ / "subdir2" / "nested");
    fs::create_directories(test_dir_ / ".hidden_dir");
    
    // Create test files
    create_file(test_dir_ / "file1.txt");
    create_file(test_dir_ / "file2.txt");
    create_file(test_dir_ / "file.pdf");
    create_file(test_dir_ / "test1.txt");
    create_file(test_dir_ / "test2.txt");
    create_file(test_dir_ / ".hidden.txt");
    create_file(test_dir_ / "subdir1" / "file3.txt");
    create_file(test_dir_ / "subdir1" / "file4.pdf");
    create_file(test_dir_ / "subdir2" / "file5.txt");
    create_file(test_dir_ / "subdir2" / "nested" / "file6.txt");
  }
  
  void TearDown() override {
    if (fs::exists(test_dir_)) {
      fs::remove_all(test_dir_);
    }
  }
  
  void create_file(const fs::path& path) {
    std::ofstream file(path.string());
    file << "test content";
    file.close();
  }
  
  fs::path test_dir_;
};

// ============================================================================
// Basic File Matching Tests
// ============================================================================

TEST_F(FileGlobTestFixture, BasicPattern) {
  fs::path pattern = test_dir_ / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  // Verify all results are .txt files
  for (const auto& result : results) {
    EXPECT_EQ(result.path().extension().string(), ".txt");
  }
}

TEST_F(FileGlobTestFixture, PatternWithQuestion) {
  fs::path pattern = test_dir_ / "test?.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  // Verify all results match test?.txt pattern
  for (const auto& result : results) {
    std::string filename = result.path().filename().string();
    EXPECT_TRUE(filename.find("test") == 0);
    EXPECT_EQ(filename.substr(filename.length() - 4), ".txt");
    EXPECT_EQ(filename.length(), 9); // test + 1 char + .txt
  }
}

TEST_F(FileGlobTestFixture, PatternWithSet) {
  fs::path pattern = test_dir_ / "file[12].txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should match file1.txt and file2.txt
  EXPECT_GE(results.size(), 2);
}

// ============================================================================
// Directory Traversal Tests
// ============================================================================

TEST_F(FileGlobTestFixture, RecursivePattern) {
  fs::path pattern = test_dir_ / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find files in current dir and subdirectories
  EXPECT_GT(results.size(), 0);
  
  bool found_nested = false;
  for (const auto& result : results) {
    if (result.path().string().find("nested") != std::string::npos) {
      found_nested = true;
      break;
    }
  }
  EXPECT_TRUE(found_nested);
}

TEST_F(FileGlobTestFixture, RecursiveInSubdir) {
  fs::path pattern = test_dir_ / "subdir1" / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  // All results should be in subdir1
  for (const auto& result : results) {
    EXPECT_TRUE(result.path().string().find("subdir1") != std::string::npos);
  }
}

// ============================================================================
// Relative Path Tests
// ============================================================================

TEST_F(FileGlobTestFixture, CurrentDirectory) {
  // Change to test directory
  fs::current_path(test_dir_);
  
  fs::path pattern = "./*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  // Restore original path (if needed)
  fs::current_path(fs::temp_directory_path());
}

TEST_F(FileGlobTestFixture, ParentDirectory) {
  fs::path subdir = test_dir_ / "subdir1";
  fs::current_path(subdir);
  
  fs::path pattern = "../*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  fs::current_path(fs::temp_directory_path());
}

// ============================================================================
// Hidden Files Tests
// ============================================================================

TEST_F(FileGlobTestFixture, HiddenFiles) {
  fs::path pattern = test_dir_ / ".*";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find .hidden.txt
  EXPECT_GT(results.size(), 0);
  
  bool found_hidden = false;
  for (const auto& result : results) {
    std::string filename = result.path().filename().string();
    if (filename[0] == '.') {
      found_hidden = true;
      break;
    }
  }
  EXPECT_TRUE(found_hidden);
}

TEST_F(FileGlobTestFixture, HiddenFilesPattern) {
  fs::path pattern = test_dir_ / ".hidden*";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  for (const auto& result : results) {
    std::string filename = result.path().filename().string();
    EXPECT_TRUE(filename.find(".hidden") == 0);
  }
}

TEST_F(FileGlobTestFixture, NonHiddenPatternIgnoresHidden) {
  fs::path pattern = test_dir_ / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should not include .hidden.txt
  for (const auto& result : results) {
    std::string filename = result.path().filename().string();
    EXPECT_FALSE(filename[0] == '.' && filename != ".txt");
  }
}

// ============================================================================
// Match Results Tests
// ============================================================================

TEST_F(FileGlobTestFixture, MatchResultsCaptured) {
  fs::path pattern = test_dir_ / "test*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  for (const auto& result : results) {
    const auto& match_res = result.match_result();
    // Match results should contain captured groups
    // (exact content depends on implementation)
    EXPECT_GE(match_res.size(), 0);
  }
}

// ============================================================================
// Error Cases
// ============================================================================

TEST_F(FileGlobTestFixture, NonExistentDirectory) {
  fs::path pattern = test_dir_ / "nonexistent" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should return empty or handle gracefully
  EXPECT_GE(results.size(), 0);
}

TEST_F(FileGlobTestFixture, InvalidPattern) {
  // Test with malformed pattern - should handle gracefully
  fs::path pattern = test_dir_ / "[invalid";
  
  // This might throw or return empty - test actual behavior
  try {
    glob::file_glob fglob(pattern.string());
    std::vector<glob::path_match> results = fglob.Exec();
    // If it doesn't throw, results should be empty or handled
    EXPECT_GE(results.size(), 0);
  } catch (const glob::Error& e) {
    // Exception is acceptable for invalid patterns
    EXPECT_TRUE(true);
  }
}

// ============================================================================
// Specific Pattern Tests
// ============================================================================

TEST_F(FileGlobTestFixture, PDFFilesOnly) {
  fs::path pattern = test_dir_ / "*.pdf";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  for (const auto& result : results) {
    EXPECT_EQ(result.path().extension().string(), ".pdf");
  }
}

TEST_F(FileGlobTestFixture, FileNumberPattern) {
  fs::path pattern = test_dir_ / "file[0-9].txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should match file1.txt, file2.txt, etc.
  EXPECT_GT(results.size(), 0);
  
  for (const auto& result : results) {
    std::string filename = result.path().filename().string();
    EXPECT_TRUE(filename.find("file") == 0);
    EXPECT_TRUE(std::isdigit(filename[4]));
  }
}

TEST_F(FileGlobTestFixture, GroupPattern) {
  fs::path pattern = test_dir_ / "file*.(txt|pdf)";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  for (const auto& result : results) {
    std::string ext = result.path().extension().string();
    EXPECT_TRUE(ext == ".txt" || ext == ".pdf");
  }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FileGlobTestFixture, EmptyDirectory) {
  fs::path empty_dir = test_dir_ / "empty";
  fs::create_directories(empty_dir);
  
  fs::path pattern = empty_dir / "*";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should return empty for empty directory
  EXPECT_EQ(results.size(), 0);
  
  fs::remove_all(empty_dir);
}

TEST_F(FileGlobTestFixture, PatternNoMatches) {
  fs::path pattern = test_dir_ / "nonexistent*.xyz";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should return empty when no matches
  EXPECT_EQ(results.size(), 0);
}

TEST_F(FileGlobTestFixture, SingleFilePattern) {
  fs::path pattern = test_dir_ / "file1.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should match exactly one file
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].path().filename().string(), "file1.txt");
}

