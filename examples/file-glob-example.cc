#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "glob-cpp/file-glob.h"

namespace fs = boost::filesystem;

void print_results(const std::string& pattern, const std::vector<glob::path_match>& results) {
  std::cout << "Pattern: \"" << pattern << "\"\n";
  std::cout << "Found " << results.size() << " file(s):\n";
  
  if (results.empty()) {
    std::cout << "  (no matches)\n";
  } else {
    for (const auto& match : results) {
      std::cout << "  " << match.path().string() << "\n";
    }
  }
  std::cout << "\n";
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    // CLI mode: single pattern from command line
    std::string pattern = argv[1];
    
    try {
      glob::file_glob fglob(pattern);
      std::vector<glob::path_match> results = fglob.Exec();
      
      print_results(pattern, results);
      return results.empty() ? 1 : 0;
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << "\n";
      return 1;
    }
  }

  // Interactive mode: demonstrate various patterns
  std::cout << "=== File Globbing Examples ===\n\n";

  // Example 1: Simple pattern in current directory
  std::cout << "Example 1: Find all .txt files in current directory\n";
  try {
    glob::file_glob fglob1("*.txt");
    std::vector<glob::path_match> results1 = fglob1.Exec();
    print_results("*.txt", results1);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  // Example 2: Recursive search with **/
  std::cout << "Example 2: Recursive search - find all .txt files at any depth\n";
  try {
    glob::file_glob fglob2("**/*.txt");
    std::vector<glob::path_match> results2 = fglob2.Exec();
    print_results("**/*.txt", results2);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  // Example 3: Recursive search with brace expansion
  std::cout << "Example 3: Recursive search with brace expansion - C/C++ header files\n";
  try {
    glob::file_glob fglob3("**/*.{h,hpp}");
    std::vector<glob::path_match> results3 = fglob3.Exec();
    print_results("**/*.{h,hpp}", results3);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  // Example 4: Pattern with character sets
  std::cout << "Example 4: Find files with numeric suffixes\n";
  try {
    glob::file_glob fglob4("**/file[0-9].txt");
    std::vector<glob::path_match> results4 = fglob4.Exec();
    print_results("**/file[0-9].txt", results4);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  // Example 5: Pattern in specific subdirectory
  std::cout << "Example 5: Find files in 'src' directory tree\n";
  try {
    glob::file_glob fglob5("src/**/*.cpp");
    std::vector<glob::path_match> results5 = fglob5.Exec();
    print_results("src/**/*.cpp", results5);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  // Example 6: Complex pattern with wildcards and sets
  std::cout << "Example 6: Complex pattern - test files with numbers\n";
  try {
    glob::file_glob fglob6("**/test*[0-9].{txt,md}");
    std::vector<glob::path_match> results6 = fglob6.Exec();
    print_results("**/test*[0-9].{txt,md}", results6);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  // Example 7: Find all files recursively
  std::cout << "Example 7: Find all files recursively\n";
  try {
    glob::file_glob fglob7("**/*");
    std::vector<glob::path_match> results7 = fglob7.Exec();
    std::cout << "Pattern: \"**/*\"\n";
    std::cout << "Found " << results7.size() << " file(s)\n";
    if (results7.size() > 0 && results7.size() <= 10) {
      for (const auto& match : results7) {
        std::cout << "  " << match.path().string() << "\n";
      }
    } else if (results7.size() > 10) {
      std::cout << "  (showing first 10)\n";
      for (size_t i = 0; i < 10; ++i) {
        std::cout << "  " << results7[i].path().string() << "\n";
      }
      std::cout << "  ... and " << (results7.size() - 10) << " more\n";
    }
    std::cout << "\n";
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n\n";
  }

  std::cout << "=== Examples Complete ===\n";
  std::cout << "\nTip: Run with a pattern argument to search for specific files:\n";
  std::cout << "  " << (argc > 0 ? argv[0] : "file-glob-example") << " \"**/*.cpp\"\n";
  
  return 0;
}

