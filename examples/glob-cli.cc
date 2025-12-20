#include <iostream>
#include <string>
#include "glob-cpp/glob.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <pattern> <string>\n";
    std::cerr << "Example: " << argv[0] << " \"*.txt\" \"file.txt\"\n";
    return 1;
  }

  std::string pattern = argv[1];
  std::string input_string = argv[2];

  glob::glob glob_pattern(pattern);
  bool matches = glob::glob_match(input_string, glob_pattern);

  std::cout << "Pattern: \"" << pattern << "\"\n";
  std::cout << "String:  \"" << input_string << "\"\n";
  std::cout << "Result:  " << (matches ? "MATCH" : "NO MATCH") << "\n";

  return matches ? 0 : 1;
}

