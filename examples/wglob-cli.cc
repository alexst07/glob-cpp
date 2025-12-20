// wglob-cli.cc
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include "glob-cpp/glob.h"

// Helper to convert UTF-8 std::string_view to std::wstring
std::wstring utf8_to_wstring(const std::string_view& utf8)
{
  // deprecated in C++17 and removed in C++26
  // does the job for now but we will need a different way to convert
  // utf8 to utf32 in the future
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(utf8.data());
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <pattern> <string>\n";
    std::cerr << "Example: " << argv[0] << " \"[a-z]*\" \"test.txt\"\n";
    std::cerr << "Both arguments are interpreted as UTF-8.\n";
    return 1;
  }

  std::string_view utf8_pattern = argv[1];
  std::string_view utf8_input   = argv[2];

  std::wstring pattern;
  std::wstring input_string;

  try {
    pattern    = utf8_to_wstring(utf8_pattern);
    input_string = utf8_to_wstring(utf8_input);
  } catch (const std::exception& e) {
    std::cerr << "Error converting UTF-8 to wchar_t: " << e.what() << "\n";
    return 1;
  }

  glob::wglob glob_pattern(pattern);
  bool matches = glob::glob_match(input_string, glob_pattern);

  std::cout << "Pattern: \"" << utf8_pattern << "\"\n";
  std::cout << "String:  \"" << utf8_input << "\"\n";
  std::cout << "Result:  " << (matches ? "MATCH" : "NO MATCH") << "\n";

  return matches ? 0 : 1;
}
