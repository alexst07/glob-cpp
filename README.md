# glob-cpp

A powerful, header-only C++ library for glob pattern matching on strings and filesystems. Supports extended glob patterns, brace expansion, and recursive directory traversal.

## Features

- **Basic Wildcards**: `*` (match any sequence) and `?` (match any single character)
- **Character Sets**: `[a-z]`, `[0-9]`, `[^abc]` (negated sets)
- **Extended Glob Groups**: `?()`, `*()`, `+()`, `@()`, `!()` for pattern repetition
- **Union Patterns**: `(a|b|c)` for matching alternatives
- **Brace Expansion**: `{a,b,c}` for shell-style pattern expansion
- **Nested Patterns**: Support for nested braces and groups
- **Escape Sequences**: `\*`, `\?`, `\{`, `\}` to match literal characters
- **File System Globbing**: Recursive directory traversal
- **Match Extraction**: Capture matched substrings from patterns
- **Wide Character Support**: `wglob` and `wmatch` for wide strings

## Quick Start

```cpp
#include "glob-cpp/glob.h"

int main() {
    glob::glob pattern("*.{h,hpp,c,cpp}");
    
    // Check if a string matches
    if (glob::glob_match("file.h", pattern)) {
        std::cout << "Match!" << std::endl;
    }
    
    return 0;
}
```

## Pattern Syntax

### Basic Wildcards

| Pattern | Description | Example Matches |
|---------|-------------|----------------|
| `*` | Matches zero or more characters | `""`, `"a"`, `"abc"`, `"any string"` |
| `?` | Matches exactly one character | `"a"`, `"1"`, `"Z"` |

**Examples:**
```cpp
glob::glob g1("*.txt");        // Matches: "file.txt", "document.txt", ".txt"
glob::glob g2("file?.txt");    // Matches: "file1.txt", "fileA.txt"
glob::glob g3("test*file");    // Matches: "testfile", "test123file"
```

### Character Sets

| Pattern | Description | Example Matches |
|---------|-------------|----------------|
| `[abc]` | Matches any character in the set | `"a"`, `"b"`, `"c"` |
| `[a-z]` | Matches any character in the range | `"a"`, `"m"`, `"z"` |
| `[^abc]` | Matches any character NOT in the set | `"d"`, `"1"`, `"Z"` |
| `[0-9A-Z]` | Multiple ranges | `"0"`, `"5"`, `"A"`, `"Z"` |

**Examples:**
```cpp
glob::glob g1("file_[0-9].txt");      // Matches: "file_1.txt", "file_5.txt"
glob::glob g2("[A-Z]*.jpg");         // Matches: "Photo.jpg", "Image.jpg"
glob::glob g3("[^a-z].txt");         // Matches: "A.txt", "1.txt" (not "a.txt")
```

### Extended Glob Groups

Extended glob patterns use parentheses with prefixes to control repetition:

| Pattern | Description | Example Matches |
|---------|-------------|----------------|
| `?(pattern)` | Zero or one occurrence | `""`, `"pattern"` |
| `*(pattern)` | Zero or more occurrences | `""`, `"pattern"`, `"patternpattern"` |
| `+(pattern)` | One or more occurrences | `"pattern"`, `"patternpattern"` |
| `@(pattern)` | Exactly one occurrence | `"pattern"` |
| `!(pattern)` | Anything except pattern | `"other"`, `"text"` (not `"pattern"`) |

**Examples:**
```cpp
glob::glob g1("*([a-z])+([0-9]).txt");  // Letters (optional) + digits (required) + .txt
                                         // Matches: "file1.txt", "123.txt", "abc123.txt"

glob::glob g2("?([A-Z])+([a-z0-9]).txt"); // Optional capital + lowercase/digits + .txt
                                           // Matches: "File1.txt", "file.txt", "123.txt"

glob::glob g3("!(*.jpg|*.gif)");        // Anything except .jpg or .gif files
```

### Union Patterns

Use `|` to match any of several alternatives:

```cpp
glob::glob g("file.(txt|pdf|doc)");  // Matches: "file.txt", "file.pdf", "file.doc"
```

**Examples:**
```cpp
glob::glob g1("(a|b|c)");                    // Matches: "a", "b", "c"
glob::glob g2("*([a-zA-Z])*([0-9]).(txt|pdf)"); // Complex union with groups
```

### Brace Expansion

Shell-style brace expansion expands patterns into multiple alternatives:

| Pattern | Expands To | Matches |
|---------|-----------|---------|
| `*.{h,hpp}` | `*.h` OR `*.hpp` | `"file.h"`, `"test.hpp"` |
| `{a,b}*.txt` | `a*.txt` OR `b*.txt` | `"a.txt"`, `"bfile.txt"` |
| `*.{h{pp,xx},c}` | `*.hpp` OR `*.hxx` OR `*.c` | `"file.hpp"`, `"file.hxx"`, `"file.c"` |

**Examples:**
```cpp
glob::glob g1("*.{h,hpp,c,cpp}");        // C/C++ source files
                                          // Matches: "file.h", "file.hpp", "file.c", "file.cpp"

glob::glob g2("test.{txt,md}");          // Documentation files
                                          // Matches: "test.txt", "test.md"

glob::glob g3("{a,b}*.txt");             // Files starting with a or b
                                          // Matches: "a.txt", "b123.txt"

glob::glob g4("*.{h{pp,xx},c}");         // Nested braces
                                          // Matches: "file.hpp", "file.hxx", "file.c"
```

**Note:** Spaces in brace expansion are significant. `*.{ged, pdf}` expands to `*.ged` and `*. pdf` (with space).

### Escape Sequences

Escape special characters to match them literally:

```cpp
glob::glob g1("\\*.txt");        // Matches literal "*" followed by ".txt"
glob::glob g2("file\\?.txt");    // Matches "file?" literally
glob::glob g3("\\{test\\}");     // Matches "{test}" literally
```

## Usage Examples

### String Matching

Check if a string matches a pattern:

```cpp
#include "glob-cpp/glob.h"

glob::glob pattern("*.pdf");
bool matches = glob::glob_match("document.pdf", pattern);
// matches == true
```

### Extract Match Results

Capture matched substrings from wildcards and groups:

```cpp
#include "glob-cpp/glob.h"

glob::glob pattern("test*.txt");
glob::cmatch results;

if (glob::glob_match("test123.txt", results, pattern)) {
    for (const auto& match : results) {
        std::cout << "Matched: " << match << std::endl;
        // Output: "123" (the part matched by *)
    }
}
```

### File System Globbing

Find files matching a pattern in directories:

```cpp
#include "glob-cpp/file-glob.h"

// Find all PDF files in current directory
glob::file_glob fglob("*.pdf");
std::vector<glob::path_match> results = fglob.Exec();

for (const auto& result : results) {
    std::cout << "Found: " << result.path() << std::endl;
    
    // Access match results for this file
    const auto& match = result.match_result();
    for (const auto& token : match) {
        std::cout << "  Captured: " << token << std::endl;
    }
}
```

### Recursive Directory Search

The file globbing API supports recursive directory traversal. Patterns are matched against file paths:

```cpp
// Find all .txt files recursively
glob::file_glob fglob("**/*.txt");
std::vector<glob::path_match> results = fglob.Exec();

// Find files matching pattern in specific directory
glob::file_glob fglob2("src/**/*.{h,hpp}");
```

Note: In file globbing, `**` is treated as a path component for recursive traversal, but in string matching patterns, multiple stars (`**`, `***`, etc.) are equivalent to a single `*`.

### Wide Character Support

For wide strings (`std::wstring`):

```cpp
glob::wglob pattern(L"*.txt");
bool matches = glob::glob_match(L"file.txt", pattern);

glob::wmatch results;
glob::wglob wpattern(L"test*.txt");
glob::glob_match(L"test123.txt", results, wpattern);
```

### Complex Patterns

Combine multiple features:

```cpp
// Match C/C++ source files with version numbers
glob::glob pattern("*([a-zA-Z])+([0-9]).{h,hpp,c,cpp}");
// Matches: "file1.h", "File123.hpp", "test5.c", "abc456.cpp"

// Match files excluding certain extensions
glob::glob pattern2("!(*.jpg|*.gif|*.png)");
// Matches anything except image files

// Complex brace expansion with sets
glob::glob pattern3("file[0-9].{txt,pdf,md}");
// Matches: "file1.txt", "file5.pdf", "file9.md"
```

## API Reference

### Basic Types

```cpp
namespace glob {
    // String matching
    using glob = basic_glob<char, extended_glob<char>>;
    using wglob = basic_glob<wchar_t, extended_glob<wchar_t>>;
    
    // Match results
    using cmatch = MatchResults<char>;
    using wmatch = MatchResults<wchar_t>;
    
    // File globbing
    using file_glob = FileGlog<char>;
    using wfile_glob = FileGlog<wchar_t>;
    using path_match = PathMatch<char>;
    using wpath_match = PathMatch<wchar_t>;
}
```

### Core Functions

```cpp
// Check if string matches pattern
bool glob_match(const std::string& str, glob::glob& pattern);
bool glob_match(const char* str, glob::glob& pattern);

// Match and capture results
bool glob_match(const std::string& str, glob::cmatch& results, glob::glob& pattern);
bool glob_match(const char* str, glob::cmatch& results, glob::glob& pattern);
```

### Error Handling

The library throws `glob::Error` exceptions for invalid patterns:

```cpp
try {
    glob::glob pattern("*.{h,hpp");  // Missing closing brace
} catch (const glob::Error& e) {
    std::cerr << "Pattern error: " << e.what() << std::endl;
}
```

## Building

### Requirements

- C++11 or later
- CMake 3.5+
- Boost.Filesystem (for file globbing)
- Google Test (for tests, optional)

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make

# Run tests
make test
# Or
./tests/glob-str-test
```

### Header-Only Usage

The library is header-only. Simply include the headers:

```cpp
#include "glob-cpp/glob.h"        // For string matching
#include "glob-cpp/file-glob.h"    // For file system globbing (requires Boost)
```

## Pattern Examples Reference

### File Extensions

```cpp
"*.{h,hpp,c,cpp}"           // C/C++ source files
"*.{txt,md,rst}"            // Documentation files
"*.{jpg,jpeg,png,gif}"     // Image files
"file[0-9].{txt,pdf}"      // Numbered files with multiple extensions
```

### Version Patterns

```cpp
"app-v[0-9].[0-9].exe"      // Version numbers: app-v1.0.exe, app-v2.5.exe
"lib*([0-9]).so"            // Libraries with optional version: lib.so, lib1.so
```

### Complex Matching

```cpp
"*([A-Z])+([a-z0-9]).txt"   // Capital letter(s) + lowercase/digits + .txt
"!(*.tmp|*.bak)"            // Everything except temporary/backup files
"test?*file"                // test + one char + anything + file
```

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please ensure tests pass and follow the existing code style.
