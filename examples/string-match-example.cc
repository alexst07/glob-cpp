#include <iostream>
#include <string>
#include <vector>
#include "glob-cpp/glob.h"

int main() {
  std::cout << "=== Glob String Matching Examples ===\n\n";

  // Example 1: Simple wildcard matching
  std::cout << "Example 1: Simple wildcard (*)\n";
  glob::glob pattern1("*.txt");
  std::vector<std::string> test_strings1 = {
    "file.txt",
    "document.txt",
    "file.pdf",
    "file.txt.bak",
    ".txt"
  };

  for (const auto& str : test_strings1) {
    bool matches = glob::glob_match(str, pattern1);
    std::cout << "  Pattern: *.txt, String: \"" << str << "\" -> "
              << (matches ? "MATCH" : "NO MATCH") << "\n";
  }

  // Example 2: Single character wildcard (?)
  std::cout << "\nExample 2: Single character wildcard (?)\n";
  glob::glob pattern2("file?.txt");
  std::vector<std::string> test_strings2 = {
    "file1.txt",
    "fileA.txt",
    "file.txt",
    "file12.txt",
    "file_.txt"
  };

  for (const auto& str : test_strings2) {
    bool matches = glob::glob_match(str, pattern2);
    std::cout << "  Pattern: file?.txt, String: \"" << str << "\" -> "
              << (matches ? "MATCH" : "NO MATCH") << "\n";
  }

  // Example 3: Character sets [a-z]
  std::cout << "\nExample 3: Character sets [a-z]\n";
  glob::glob pattern3("file_[0-9].txt");
  std::vector<std::string> test_strings3 = {
    "file_1.txt",
    "file_5.txt",
    "file_0.txt",
    "file_10.txt",
    "file_a.txt"
  };

  for (const auto& str : test_strings3) {
    bool matches = glob::glob_match(str, pattern3);
    std::cout << "  Pattern: file_[0-9].txt, String: \"" << str << "\" -> "
              << (matches ? "MATCH" : "NO MATCH") << "\n";
  }

  // Example 4: Combined patterns
  std::cout << "\nExample 4: Combined patterns (* and ?)\n";
  glob::glob pattern4("test?*.txt");
  std::vector<std::string> test_strings4 = {
    "test1.txt",
    "testA.txt",
    "test_file.txt",
    "test.txt",
    "test123.txt"
  };

  for (const auto& str : test_strings4) {
    bool matches = glob::glob_match(str, pattern4);
    std::cout << "  Pattern: test?*.txt, String: \"" << str << "\" -> "
              << (matches ? "MATCH" : "NO MATCH") << "\n";
  }

  // Example 5: Extended glob with groups
  std::cout << "\nExample 5: Extended glob with groups\n";
  glob::glob pattern5("*([a-zA-Z])+([0-9]).txt");
  std::vector<std::string> test_strings5 = {
    "file1.txt",
    "file12.txt",
    "FILE123.txt",
    "file.txt",
    "123.txt"
  };

  for (const auto& str : test_strings5) {
    bool matches = glob::glob_match(str, pattern5);
    std::cout << "  Pattern: *([a-zA-Z])+([0-9]).txt, String: \"" << str << "\" -> "
              << (matches ? "MATCH" : "NO MATCH") << "\n";
  }

  // Example 6: Union pattern
  std::cout << "\nExample 6: Union pattern (|)\n";
  glob::glob pattern6("file.(txt|pdf|doc)");
  std::vector<std::string> test_strings6 = {
    "file.txt",
    "file.pdf",
    "file.doc",
    "file.jpg",
    "file.png"
  };

  for (const auto& str : test_strings6) {
    bool matches = glob::glob_match(str, pattern6);
    std::cout << "  Pattern: file.(txt|pdf|doc), String: \"" << str << "\" -> "
              << (matches ? "MATCH" : "NO MATCH") << "\n";
  }

  std::cout << "\n=== Examples Complete ===\n";
  return 0;
}

