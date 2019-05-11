#ifndef FILE_GLOB_CPP_H
#define FILE_GLOB_CPP_H

#include <iostream>
#include "glob.h"

namespace glob {

namespace fs = boost::filesystem;

template<class charT>
class PathMatch {
 public:
  PathMatch(fs::path&& path, MatchResults<charT>&& match_res)
      : path_{std::move(path)}
      , match_res_{std::move(match_res)} {}

  PathMatch(const fs::path& path, MatchResults<charT>&& match_res)
      : path_{path}
      , match_res_{std::move(match_res)} {}

  PathMatch(const PathMatch& pm)
      : path_{pm.path_}
      , match_res_{pm.match_res_} {}

  PathMatch(PathMatch&& pm)
      : path_{std::move(pm.path_)}
      , match_res_{std::move(pm.match_res_)} {}

  PathMatch& operator=(const PathMatch& pm) {
    path_ = pm.path_;
    match_res_ = pm.match_res_;
  }

  PathMatch& operator=(PathMatch&& pm) {
    path_ = std::move(pm.path_);
    match_res_ = std::move(pm.match_res_);
  }

  const fs::path path() const {
    return path_;
  }

  const MatchResults<charT>& match_result() const {
    return match_res_;
  }

 private:
  fs::path path_;
  MatchResults<charT> match_res_;
};

template<class charT>
class FileGlog {
 public:
  FileGlog(const String<charT>& str_path): path_{str_path} {}

  std::vector<PathMatch<charT>> Exec() {
    std::vector<String<charT>> vec_glob_path;
      for (auto it = path_.begin(); it != path_.end(); it++ ) {
        vec_glob_path.push_back(it->string());
      }

      size_t level = 0;
      std::vector<PathMatch<charT>> vec_files;
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

      return std::vector<PathMatch<charT>>{};
  }

 private:
  std::vector<PathMatch<charT>> HandleRootDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<PathMatch<charT>> vec_ret;
    fs::path p{"/"};

    if (vec_glob_path.size() < 2) {
      return std::vector<PathMatch<charT>>{};
    }

    if (IsParentDir(vec_glob_path[1])) {
      return std::vector<PathMatch<charT>>{};
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

  std::vector<PathMatch<charT>> HandleHomeDir(
      const std::vector<String<charT>>& vec_glob_path) {
    namespace bp = boost::process;

    std::vector<PathMatch<charT>> vec_ret;
    bp::environment env = boost::this_process::environment();
    fs::path p{env["HOME"].to_string()};

    if (vec_glob_path.size() < 2) {
      return std::vector<PathMatch<charT>>{PathMatch<charT>{std::move(p),
          MatchResults<charT>{}}};
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

  std::vector<PathMatch<charT>> HandleUpDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<PathMatch<charT>> vec_ret;
    fs::path p{".."};

    if (vec_glob_path.size() < 2) {
      return std::vector<PathMatch<charT>>{};
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

  std::vector<PathMatch<charT>> HandleThisDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<PathMatch<charT>> vec_ret;
    fs::path p{"."};

    if (vec_glob_path.size() < 2) {
      return std::vector<PathMatch<charT>>{};
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

  std::vector<PathMatch<charT>> HandleDir(
      const std::vector<String<charT>>& vec_glob_path) {
    std::vector<PathMatch<charT>> vec_ret;
    fs::path p{"."};

    if (vec_glob_path.size() < 1) {
      return std::vector<PathMatch<charT>>{};
    }

    return RecursiveGlobDir(vec_glob_path, p, 0);
  }

  std::vector<PathMatch<charT>> RecursiveGlobDir(
      const std::vector<String<charT>>& vec_glob_path,
      const fs::path& real_path,
      int level) {
    std::vector<PathMatch<charT>> vec_ret;
    fs::path p = real_path;
    if (level >= vec_glob_path.size()) {
      return std::vector<PathMatch<charT>>{};
    }

    if (level == (vec_glob_path.size() - 1)) {
      if (IsParentDir(vec_glob_path[level])) {
        p /= fs::path{".."};
        return std::vector<PathMatch<charT>>{PathMatch<charT>{std::move(p),
          MatchResults<charT>{}}};
      }

      if (IsThisDir(vec_glob_path[level])) {
        p /= fs::path{"."};
        return std::vector<PathMatch<charT>>{PathMatch<charT>{std::move(p),
          MatchResults<charT>{}}};
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
      MatchResults<charT> match_res;
      if (glob_match(d.path().filename().string(), match_res, g)) {
        if (level == (vec_glob_path.size() - 1)) {
          if (IsHidden(d.path()) && vec_glob_path[level][0] != '.') {
            continue;
          }

          PathMatch<charT> path_match(d.path(), std::move(match_res));
          vec_ret.push_back(std::move(path_match));
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

  std::vector<PathMatch<charT>> TwoStarsGlobDir(
      const std::vector<String<charT>>& vec_glob_path,
      const fs::path& real_path,
      int level) {
    fs::recursive_directory_iterator end;
    std::vector<PathMatch<charT>> vec_paths;

    for (fs::recursive_directory_iterator it(real_path); it != end; ++it) {
      MatchGlobDir(vec_glob_path, real_path, *it, level, vec_paths);
    }

    return vec_paths;
  }

  bool MatchGlobDir(const std::vector<String<charT>>& vec_glob_path,
      const fs::path& base_path, const fs::path& real_path,
      int level, std::vector<PathMatch<charT>>& vec_res) {
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
    MatchResults<charT> match_res;

    for (size_t i = vec_glob_path.size(); i > level; i--) {
      glob g(vec_glob_path[glob_size - j]);
      if (!glob_match(vec_path[real_path_size - j], match_res, g)) {
        return false;
      }

      j++;
    }

    PathMatch<charT> path_res{real_path, std::move(match_res)};
    vec_res.push_back(std::move(path_res));
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

using path_match = PathMatch<char>;
using wpath_match = PathMatch<wchar_t>;
using file_glob = FileGlog<char>;
using wfile_glob = FileGlog<wchar_t>;
}

#endif
