# glob-cpp

Glob CPP is a header only library based on STL regular expression to perform glob operation on string, or on filesystem.

```
?(pattern-list)   Matches zero or one occurrence of the given patterns
*(pattern-list)   Matches zero or more occurrences of the given patterns
+(pattern-list)   Matches one or more occurrences of the given patterns
@(pattern-list)   Matches one of the given patterns
!(pattern-list)   Matches anything except one of the given patterns
```
## Glob Examples
```
*.jpg         : All JPEG files
[A-Z]*.jpg    : JPEG files that start with a capital letter
*!(.jpg|.gif) : All files, except JPEGs or GIFs.
```

## Examples
### Match with string
Verify is a given string match with glob expression.
```cpp
#include "glob.h"

int main () {
  glob::glob g("*.pdf");
  bool r = glob_match("test.pdf", g);
  std::cout << "match: " << r?"yes":"no" << "\n";
  return 0;
}
```

### Get match substrings
Print the matches found on the target sequence of characters after a glob matching operation.
```cpp
#include "glob.h"

int main () {
  glob::glob g("*.pdf");
  glob::cmatch m;
  if (glob_match("test.pdf", m, g)) {
    for (auto& token : m) {
      std::cout << "sub string: " << token << "\n";
    }
  }
  
  return 0;
}
```

### Get files from match operation in a directory and all match substrings
Given a directory, this example list all files that match with the glob expression. For example:
`*.pdf` get all pdf files in the directory, and `**/*.pdf` get all pdf files in all sub directories.

```cpp
#include "file-glob.h"

int main () {
  glob::file_glob<char> fglob{argv[1]};
  std::vector<path_match> results = fglob.Exec();

  for (auto& res : results) {
    std::cout << "path: " <<  res.path() << "[";
    auto& match_res = res.match_result();
    for (auto& token : match_res) {
      std::cout << " \"" << token << "\" ";
    }
    std::cout << "]" << std::endl;
  }
  
  return 0;
}
```
