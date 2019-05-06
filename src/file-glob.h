#ifndef FILE_GLOB_CPP_H
#define FILE_GLOB_CPP_H

#include <iostream>
#include "glob.h"

namespace glob {

namespace fs = boost::filesystem;

template<class charT>
class FileGlog {
 public:
  FileGlog(const String<charT>& str_path): path_{str_path} {}

  std::vector<fs::path> Exec() {
    std::vector<String<charT>> vec_glob_path;
      for (auto it = path_.begin(); it != path_.end(); it++ ) {
        vec_glob_path.push_back(it->string());
      }

      size_t level = 0;
      std::vector<fs::path> vec_files;
      if (IsRootDir(vec_glob_path[0])) {
        return HandleRootDir(vec_glob_path);
      } else if (IsHomeDir(vec_glob_path[0])) {
        return HandleHomeDir(vec_glob_path);
      } else if (IsParentDir(vec_glob_path[0])) {
        return HandleUpDir(vec_glob_path);
      } else if (IsThisDir(vec_glob_path[0])) {
        return HandleThisDir(vec_glob_path);
      } else if (IsTwoStarDir(vec_glob_path[0])) {
        fs::path p{"."};
        return TwoStarsGlobDir(vec_glob_path, p, 1);
      } else {
        return HandleDir(vec_glob_path);
      }

      return std::vector<fs::path>{};
  }

 private:
  std::vector<fs::path> HandleRootDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<fs::path> vec_ret;
    fs::path p{"/"};

    if (vec_glob_path.size() < 2) {
      return std::vector<fs::path>{};
    }

    if (IsParentDir(vec_glob_path[1])) {
      return std::vector<fs::path>{};
    }

    if (IsThisDir(vec_glob_path[1])) {
      p /= fs::path{"."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsTwoStarDir(vec_glob_path[1])) {
      return TwoStarsGlobDir(vec_glob_path, p, 2);
    }

    return RecursiveGlobDir(vec_glob_path, p, 1);
  }

  std::vector<fs::path> HandleHomeDir(
      const std::vector<String<charT>>& vec_glob_path) {
    namespace bp = boost::process;

    std::vector<fs::path> vec_ret;
    bp::environment env = boost::this_process::environment();
    fs::path p{env["HOME"].to_string()};

    if (vec_glob_path.size() < 2) {
      return std::vector<fs::path>{p};
    }

    if (IsParentDir(vec_glob_path[1])) {
      p /= fs::path{".."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsThisDir(vec_glob_path[1])) {
      p /= fs::path{"."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsTwoStarDir(vec_glob_path[1])) {
      return TwoStarsGlobDir(vec_glob_path, p, 2);
    }

    return RecursiveGlobDir(vec_glob_path, p, 1);
  }

  std::vector<fs::path> HandleUpDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<fs::path> vec_ret;
    fs::path p{".."};

    if (vec_glob_path.size() < 2) {
      return std::vector<fs::path>{};
    }

    if (IsParentDir(vec_glob_path[1])) {
      p /= fs::path{".."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsThisDir(vec_glob_path[1])) {
      p /= fs::path{"."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsTwoStarDir(vec_glob_path[1])) {
      return TwoStarsGlobDir(vec_glob_path, p, 2);
    }

    return RecursiveGlobDir(vec_glob_path, p, 1);
  }

  std::vector<fs::path> HandleThisDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<fs::path> vec_ret;
    fs::path p{"."};

    if (vec_glob_path.size() < 2) {
      return std::vector<fs::path>{};
    }

    if (IsParentDir(vec_glob_path[1])) {
      p /= fs::path{".."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsThisDir(vec_glob_path[1])) {
      p /= fs::path{"."};
      return RecursiveGlobDir(vec_glob_path, p, 2);
    }

    if (IsTwoStarDir(vec_glob_path[1])) {
      return TwoStarsGlobDir(vec_glob_path, p, 2);
    }

    return RecursiveGlobDir(vec_glob_path, p, 1);
  }

  std::vector<fs::path> HandleDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<fs::path> vec_ret;
    fs::path p{"."};

    if (vec_glob_path.size() < 1) {
      return std::vector<fs::path>{};
    }

    return RecursiveGlobDir(vec_glob_path, p, 0);
  }

  std::vector<fs::path> RecursiveGlobDir(
      const std::vector<String<charT>>& vec_glob_path,
      const fs::path& real_path,
      int level) {
    std::vector<fs::path> vec_ret;
    fs::path p = real_path;
    if (level >= vec_glob_path.size()) {
      return std::vector<fs::path>{};
    }

    if (level == (vec_glob_path.size() - 1)) {
      if (IsParentDir(vec_glob_path[level])) {
        p /= fs::path{".."};
        return std::vector<fs::path>{p};
      }

      if (IsThisDir(vec_glob_path[level])) {
        p /= fs::path{"."};
        return std::vector<fs::path>{p};
      }
    }

    if (IsThisDir(vec_glob_path[level])) {
      p /= fs::path{"."};
      return RecursiveGlobDir(vec_glob_path, p, level + 1);
    }

    if (IsParentDir(vec_glob_path[level])) {
      p /= fs::path{".."};
      return RecursiveGlobDir(vec_glob_path, p, level + 1);
    }

    if (IsTwoStarDir(vec_glob_path[level])) {
      return TwoStarsGlobDir(vec_glob_path, p, level + 1);
    }

    for(auto& d : boost::make_iterator_range(fs::directory_iterator(p), {})) {
      glob g(vec_glob_path[level]);
      if (glob_match(d.path().filename().string(), g)) {
        if (level == (vec_glob_path.size() - 1)) {
          if (IsHidden(d.path()) && vec_glob_path[level][0] != '.') {
            continue;
          }
          vec_ret.push_back(d.path());
        } else {
          if (!fs::is_directory(d.path()) || !HasPermission(d.path())) {
            continue;
          }
          if (IsHidden(d.path()) && vec_glob_path[level][0] != '.') {
            continue;
          }
          auto ret = RecursiveGlobDir(vec_glob_path, d.path(), level + 1);
          vec_ret.insert(vec_ret.end(), ret.begin(), ret.end());
        }
      }
    }

    return vec_ret;
  }

  std::vector<fs::path> TwoStarsGlobDir(
      const std::vector<String<charT>>& vec_glob_path,
      const fs::path& real_path,
      int level) {
    fs::recursive_directory_iterator end;
    std::vector<fs::path> vec_paths;

    for (fs::recursive_directory_iterator it(real_path); it != end; ++it) {
      if (MatchGlobDir(vec_glob_path, real_path, *it, level)) {
        vec_paths.push_back(*it);
      }
    }

    return vec_paths;
  }

  bool MatchGlobDir(const std::vector<String<charT>>& vec_glob_path,
      const fs::path& base_path, const fs::path& real_path,
      int level) {
    std::vector<String<charT>> vec_path;
    std::vector<String<charT>> vec_base_path;

    for (auto it = real_path.begin(); it != real_path.end(); it++ ) {
      vec_path.push_back(it->string());
    }

    for (auto it = base_path.begin(); it != base_path.end(); it++ ) {
      vec_base_path.push_back(it->string());
    }

    int stop_point = vec_path.size() - vec_base_path.size();
    int glob_point = vec_glob_path.size() - level;

    if (glob_point > stop_point) {
      return false;
    }

    size_t j = 1;
    size_t glob_size = vec_glob_path.size();
    size_t real_path_size = vec_path.size();

    for (size_t i = vec_glob_path.size(); i > level; i--) {
      glob g(vec_glob_path[glob_size - j]);
      if (!glob_match(vec_path[real_path_size - j], g)) {
        return false;
      }

      j++;
    }

    return true;
  }

  bool IsTwoStarDir(const String<charT>& dir) {
    if (dir.length() == 2) {
      if (dir[0] == '*' && dir[1] == '*') {
        return true;
      }
    }

    return false;
  }


  bool IsParentDir(const String<charT>& dir) {
    if (dir.length() == 2) {
      if (dir[0] == '.' && dir[1] == '.') {
        return true;
      }
    }

    return false;
  }

  bool IsThisDir(const String<charT>& dir) {
    if (dir.length() == 1) {
      if (dir[0] == '.') {
        return true;
      }
    }

    return false;
  }

  bool IsRootDir(const String<charT>& dir) {
    if (dir.length() == 1) {
      if (dir[0] == '/') {
        return true;
      }
    }

    return false;
  }

  bool IsHomeDir(const String<charT>& dir) {
    if (dir.length() == 1) {
      if (dir[0] == '~') {
        return true;
      }
    }

    return false;
  }

  bool HasPermission(const fs::path& path) {
    std::ifstream ifs (path.string(), std::ifstream::in);
    return ifs.good();
  }

  bool IsHidden(const fs::path& path) {
    fs::path::string_type name = path.filename().string();
    return name[0] == '.';
  }

  template<class Iterator>
  fs::path MountPath(Iterator first, Iterator last) {
    fs::path path;
    while (first != last) {
      path /= *first;
      ++first;
    }

    return path;
  }

  fs::path path_;
};

}

#endif
