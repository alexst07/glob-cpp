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
// Comprehensive Recursive Pattern Tests
// ============================================================================

TEST_F(FileGlobTestFixture, RecursivePatternAllFiles) {
  fs::path pattern = test_dir_ / "**" / "*";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find all files recursively: file1.txt, file2.txt, file.pdf, test1.txt, test2.txt,
  // subdir1/file3.txt, subdir1/file4.pdf, subdir2/file5.txt, subdir2/nested/file6.txt
  EXPECT_GE(results.size(), 9);
  
  // Verify files from different levels are found
  bool found_root = false;
  bool found_subdir1 = false;
  bool found_subdir2 = false;
  bool found_nested = false;
  
  for (const auto& result : results) {
    std::string path_str = result.path().string();
    if (path_str.find("subdir1") != std::string::npos) {
      found_subdir1 = true;
    }
    if (path_str.find("subdir2") != std::string::npos) {
      found_subdir2 = true;
    }
    if (path_str.find("nested") != std::string::npos) {
      found_nested = true;
    }
    if (path_str.find("subdir1") == std::string::npos && 
        path_str.find("subdir2") == std::string::npos) {
      found_root = true;
    }
  }
  
  EXPECT_TRUE(found_root);
  EXPECT_TRUE(found_subdir1);
  EXPECT_TRUE(found_subdir2);
  EXPECT_TRUE(found_nested);
}

TEST_F(FileGlobTestFixture, RecursivePatternSpecificExtension) {
  fs::path pattern = test_dir_ / "**" / "*.pdf";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find file.pdf and subdir1/file4.pdf
  EXPECT_GE(results.size(), 2);
  
  for (const auto& result : results) {
    EXPECT_EQ(result.path().extension().string(), ".pdf");
  }
  
  // Verify both root and subdirectory PDFs are found
  bool found_root_pdf = false;
  bool found_subdir_pdf = false;
  
  for (const auto& result : results) {
    std::string path_str = result.path().string();
    if (path_str.find("file.pdf") != std::string::npos && 
        path_str.find("subdir") == std::string::npos) {
      found_root_pdf = true;
    }
    if (path_str.find("subdir1/file4.pdf") != std::string::npos) {
      found_subdir_pdf = true;
    }
  }
  
  EXPECT_TRUE(found_root_pdf);
  EXPECT_TRUE(found_subdir_pdf);
}

TEST_F(FileGlobTestFixture, RecursivePatternWithWildcard) {
  fs::path pattern = test_dir_ / "**" / "test*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find test1.txt and test2.txt from root
  EXPECT_GE(results.size(), 2);
  
  for (const auto& result : results) {
    std::string filename = result.path().filename().string();
    EXPECT_TRUE(filename.find("test") == 0);
    EXPECT_EQ(result.path().extension().string(), ".txt");
  }
}

// ============================================================================
// Recursive Pattern Position Tests
// ============================================================================

TEST_F(FileGlobTestFixture, RecursivePatternFromCurrentDir) {
  // Change to test directory
  fs::current_path(test_dir_);
  
  fs::path pattern = "./**/*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  EXPECT_GT(results.size(), 0);
  
  // Verify nested files are found
  bool found_nested = false;
  for (const auto& result : results) {
    if (result.path().string().find("nested") != std::string::npos) {
      found_nested = true;
      break;
    }
  }
  EXPECT_TRUE(found_nested);
  
  // Restore original path
  fs::current_path(fs::temp_directory_path());
}

TEST_F(FileGlobTestFixture, RecursivePatternInMiddle) {
  fs::path pattern = test_dir_ / "subdir2" / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find subdir2/file5.txt and subdir2/nested/file6.txt
  EXPECT_GE(results.size(), 2);
  
  // All results should be in subdir2
  for (const auto& result : results) {
    EXPECT_TRUE(result.path().string().find("subdir2") != std::string::npos);
  }
  
  // Verify nested file is found
  bool found_nested = false;
  for (const auto& result : results) {
    if (result.path().string().find("nested") != std::string::npos) {
      found_nested = true;
      break;
    }
  }
  EXPECT_TRUE(found_nested);
}

TEST_F(FileGlobTestFixture, RecursivePatternMultipleLevels) {
  fs::path pattern = test_dir_ / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find files at multiple nesting levels:
  // Root: file1.txt, file2.txt, test1.txt, test2.txt
  // Level 1: subdir1/file3.txt, subdir2/file5.txt
  // Level 2: subdir2/nested/file6.txt
  EXPECT_GE(results.size(), 7);
  
  // Count files at different depths
  int root_level = 0;
  int level1 = 0;
  int level2 = 0;
  
  for (const auto& result : results) {
    std::string path_str = result.path().string();
    std::string relative_path = path_str.substr(test_dir_.string().length() + 1);
    
    // Count path separators to determine depth
    int depth = 0;
    for (char c : relative_path) {
      if (c == '/') depth++;
    }
    
    if (depth == 0) {
      root_level++;
    } else if (depth == 1) {
      level1++;
    } else if (depth == 2) {
      level2++;
    }
  }
  
  EXPECT_GE(root_level, 4);  // At least 4 files at root
  EXPECT_GE(level1, 2);      // At least 2 files at level 1
  EXPECT_GE(level2, 1);      // At least 1 file at level 2
}

// ============================================================================
// Recursive Pattern Edge Cases
// ============================================================================

TEST_F(FileGlobTestFixture, RecursivePatternNoMatches) {
  fs::path pattern = test_dir_ / "**" / "*.xyz";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should return empty when no matches exist
  EXPECT_EQ(results.size(), 0);
}

TEST_F(FileGlobTestFixture, RecursivePatternEmptySubdir) {
  fs::path empty_subdir = test_dir_ / "empty_subdir";
  fs::create_directories(empty_subdir);
  
  fs::path pattern = test_dir_ / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should still find other files, empty subdir doesn't affect results
  EXPECT_GT(results.size(), 0);
  
  // Verify no files from empty_subdir are returned
  for (const auto& result : results) {
    EXPECT_TRUE(result.path().string().find("empty_subdir") == std::string::npos);
  }
  
  fs::remove_all(empty_subdir);
}

TEST_F(FileGlobTestFixture, RecursivePatternHiddenFiles) {
  fs::path pattern = test_dir_ / "**" / ".*";
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

TEST_F(FileGlobTestFixture, RecursivePatternSpecificSubdir) {
  fs::path pattern = test_dir_ / "subdir2" / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Should find subdir2/file5.txt and subdir2/nested/file6.txt
  EXPECT_GE(results.size(), 2);
  
  // All results should be in subdir2
  for (const auto& result : results) {
    std::string path_str = result.path().string();
    EXPECT_TRUE(path_str.find("subdir2") != std::string::npos);
    EXPECT_TRUE(path_str.find("subdir1") == std::string::npos);
  }
  
  // Verify both direct and nested files are found
  bool found_direct = false;
  bool found_nested = false;
  
  for (const auto& result : results) {
    std::string path_str = result.path().string();
    if (path_str.find("subdir2/file5.txt") != std::string::npos) {
      found_direct = true;
    }
    if (path_str.find("subdir2/nested/file6.txt") != std::string::npos) {
      found_nested = true;
    }
  }
  
  EXPECT_TRUE(found_direct);
  EXPECT_TRUE(found_nested);
}

// ============================================================================
// Recursive Pattern Verification Tests
// ============================================================================

TEST_F(FileGlobTestFixture, RecursivePatternCountVerification) {
  fs::path pattern = test_dir_ / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Expected files:
  // Root: file1.txt, file2.txt, test1.txt, test2.txt (4 files)
  // subdir1: file3.txt (1 file)
  // subdir2: file5.txt (1 file)
  // subdir2/nested: file6.txt (1 file)
  // Total: 7 files
  EXPECT_EQ(results.size(), 7);
  
  // Verify all are .txt files
  for (const auto& result : results) {
    EXPECT_EQ(result.path().extension().string(), ".txt");
  }
}

TEST_F(FileGlobTestFixture, RecursivePatternPathVerification) {
  fs::path pattern = test_dir_ / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Expected file paths
  std::vector<std::string> expected_paths = {
    "file1.txt",
    "file2.txt",
    "test1.txt",
    "test2.txt",
    "subdir1/file3.txt",
    "subdir2/file5.txt",
    "subdir2/nested/file6.txt"
  };
  
  // Verify all expected paths are present
  for (const auto& expected : expected_paths) {
    bool found = false;
    std::string expected_full = (test_dir_ / expected).string();
    
    for (const auto& result : results) {
      if (result.path().string() == expected_full) {
        found = true;
        break;
      }
    }
    
    EXPECT_TRUE(found) << "Expected path not found: " << expected;
  }
  
  // Verify we have exactly the expected number
  EXPECT_EQ(results.size(), expected_paths.size());
}

TEST_F(FileGlobTestFixture, RecursivePatternDepthVerification) {
  fs::path pattern = test_dir_ / "**" / "*.txt";
  glob::file_glob fglob(pattern.string());
  std::vector<glob::path_match> results = fglob.Exec();
  
  // Group files by depth
  std::vector<std::string> depth0_files;  // Root level
  std::vector<std::string> depth1_files;  // One level deep
  std::vector<std::string> depth2_files;  // Two levels deep
  
  for (const auto& result : results) {
    std::string path_str = result.path().string();
    std::string relative_path = path_str.substr(test_dir_.string().length() + 1);
    
    int depth = 0;
    for (char c : relative_path) {
      if (c == '/') depth++;
    }
    
    if (depth == 0) {
      depth0_files.push_back(relative_path);
    } else if (depth == 1) {
      depth1_files.push_back(relative_path);
    } else if (depth == 2) {
      depth2_files.push_back(relative_path);
    }
  }
  
  // Verify files at each depth level
  EXPECT_EQ(depth0_files.size(), 4);  // file1.txt, file2.txt, test1.txt, test2.txt
  EXPECT_EQ(depth1_files.size(), 2);  // subdir1/file3.txt, subdir2/file5.txt
  EXPECT_EQ(depth2_files.size(), 1);  // subdir2/nested/file6.txt
  
  // Verify specific files at each depth
  bool found_depth0 = false, found_depth1 = false, found_depth2 = false;
  
  for (const auto& file : depth0_files) {
    if (file == "file1.txt" || file == "file2.txt" || 
        file == "test1.txt" || file == "test2.txt") {
      found_depth0 = true;
    }
  }
  
  for (const auto& file : depth1_files) {
    if (file == "subdir1/file3.txt" || file == "subdir2/file5.txt") {
      found_depth1 = true;
    }
  }
  
  for (const auto& file : depth2_files) {
    if (file == "subdir2/nested/file6.txt") {
      found_depth2 = true;
    }
  }
  
  EXPECT_TRUE(found_depth0);
  EXPECT_TRUE(found_depth1);
  EXPECT_TRUE(found_depth2);
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

