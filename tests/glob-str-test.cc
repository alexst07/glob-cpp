#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include "glob.h"
#include "file-glob.h"
#include "traversal.h"

bool GlobMatch(const std::string& pattern, const std::string& str) {
  size_t px = 0;
  size_t nx = 0;
  size_t next_px = 0;
  size_t next_nx = 0;

  while (px < pattern.length() || nx < str.length()) {
    if (px < pattern.length()) {
      char c = pattern[px];
      switch (c) {
        case '?': {
          if (nx < str.length()) {
            ++nx;
            ++px;
            continue;
          }
          break;
        }

        case '*': {
          next_px = px;
          next_nx = nx + 1;
          ++px;
          continue;
          break;
        }

        default: {
          if (nx < str.length() && str[nx] == c) {
            ++px;
            ++nx;
            continue;
          }
          break;
        }
      }
    }

    if (0 < next_nx && next_nx <= str.length()) {
			px = next_px;
			nx = next_nx;
			continue;
		}
    return false;
  }

  return true;
}

// bool TestGlob(const std::string& pattern, const std::string& str) {
//   SimpleGlob<char> glob;
//   glob.Parser(pattern);
//   bool r;
//   size_t pos;
//   std::tie(r, pos) = glob.GetAutomata().Exec(str);
//   std::cout << "pos: " << pos << "\n";
//   return r;
// }

// bool TestGlob2(const std::string& pattern, const std::string& str) {
//   Glob<char> glob(pattern);
//   return glob.Exec(str);
// }

// bool TestGlob3(const std::wstring& pattern, const std::wstring& str) {
//   Glob<wchar_t> glob(pattern);
//   return glob.Exec(str);
// }

// void PrintTokens(const std::string& str) {
//   Lexer<char> l(str);
//   std::vector<Token<char>> tokens = l.Scanner();
//   for (auto& token : tokens) {
//     std::cout << token << " ";
//   }
//   std::cout << "\n";
// }

int test(int argc, char **argv) {
  // std::cout << "'te*', 'teste' -> " << GlobMatch("te*", "teste") << std::endl;
  // std::cout << "'tea*', 'teste' -> " << GlobMatch("tea*", "teste") << std::endl;
  // std::cout << "'test?', 'teste' -> " << GlobMatch("test?", "teste") << std::endl;
  // std::cout << "'te?te', 'texte' -> " << GlobMatch("te?te", "texte") << std::endl;
  // std::cout << "'te?*', 'te' -> " << GlobMatch("te?*", "te") << std::endl;
  // std::cout << "'te?*', 'tesa' -> " << GlobMatch("te?*", "tesa") << std::endl;
  // std::cout << "'teste*', 'teste1' -> " << GlobMatch("te?*", "tesa") << std::endl;
  // std::cout << "'teste*', 'teste' -> " << GlobMatch("te?*", "tesa") << std::endl;
  // std::cout << "'tes*a*c*', 'teste_xfdsr_crdsd' -> " << GlobMatch("tes*a*c*", "teste_xfdsr_crdsd") << std::endl;
  // std::cout << "'tes*a*c*', 'teste_afdsr_crdsd' -> " << GlobMatch("tes*a*c*", "teste_afdsr_crdsd") << std::endl;
  // std::cout << "'te**te', 'tete' -> " << GlobMatch("te**te", "tete") << std::endl;
  // std::cout << "-----------------------------------\n";
  // std::cout << "'te', 'teste' -> " << TestGlob("te", "teste") << std::endl;
  // std::cout << "'tea*', 'teste' -> " << TestGlob("tea*", "teste") << std::endl;
  // std::cout << "'test?', 'teste' -> " << TestGlob("test?", "teste") << std::endl;
  // std::cout << "'te?te', 'texte' -> " << TestGlob("te?te", "texte") << std::endl;
  // std::cout << "'te?*', 'te' -> " << TestGlob("te?*", "te") << std::endl;
  // std::cout << "'te?*', 'tesa' -> " << TestGlob("te?*", "tesa") << std::endl;
  // std::cout << "'teste*', 'teste1' -> " << TestGlob("te?*", "tesa") << std::endl;
  // std::cout << "'teste*', 'teste' -> " << TestGlob("te?*", "tesa") << std::endl;
  // std::cout << "'tes*a*c*', 'teste_xfdsr_crdsd' -> " << TestGlob("tes*a*c*", "teste_xfdsr_crdsd") << std::endl;
  // std::cout << "'tes*a*c*', 'teste_afdsr_crdsd' -> " << TestGlob("tes*a*c*", "teste_afdsr_crdsd") << std::endl;
  // std::cout << "'te**te', 'tete' -> " << TestGlob("te**te", "tete") << std::endl;

  // PrintTokens("te*e");
  // PrintTokens("te+[a-z]?sd");
  // PrintTokens("te+[^a-z]");
  // PrintTokens("te*(as|sd[jpg])");

  // PrintAst("t*e?*[ABa-z]ar[^x]e*x");
  // PrintAst("as(se|t(se)s|e[a-z]s)");
  // PrintAst("!(+(ab|def)*+(.jpg|.gif))");
  // PrintAst("+([a-z]).(jpg|jpeg)");
  // PrintAst("*.(\\*jpg|[Pp][Nn][Gg\\?])");
  // PrintAst("[a-zA-Z0-9%]?");
  // PrintAst("+((a)|ts|s)");
  // PrintAst("+(a)");
  // PrintAst("*-[0-9].jtl");
  // std::cout << "'te*t(x)', 'teste' -> " << TestGlob2("t(e[a-f])", "tes") << std::endl;
  // std::cout << "'te*t(x)', 'teste' -> " << TestGlob2("*([a-z])w", "tw") << std::endl;
  // glob::glob g("*.pdf");
  // std::cout << "match: " << glob::glob_match("file.pdf", g) << "\n";

  namespace fs = boost::filesystem;

  // path p(argc>1? argv[1] : ".");

  // if(is_directory(p)) {
  //   std::cout << p << " is a directory containing:\n";

  //   for(auto& entry : boost::make_iterator_range(directory_iterator(p), {}))
  //       std::cout << entry << "\n";
  // }

  // auto path = fs::path("/home/**/books");
  // for (auto it = path.begin(); it != path.end(); it++ ) {
  //   std::cout << *it << "\n";
  // }

  // fs::path path;
  // path /= "~/teste";
  // path /= "teste2";

  // std::cout << path << "\n";

  fs::path p{argv[1]};
  glob::FileGlog<char> fglob{argv[1]};
  std::vector<glob::PathMatch<char>> results = fglob.Exec();

  for (auto& res : results) {
    std::cout << "path: " <<  res.path() << "[";
    auto& match_res = res.match_result();
    for (auto& token : match_res) {
      std::cout << "-" << token;
    }
    std::cout << "]" << std::endl;
  }

  // glob::glob g("+([a-z0-9]).*");
  // std::cout << "match: " << glob::glob_match("file1.pdf", g) << "\n";

  // auto vec = g.GetAutomata().GetMatchedStrings();

  // for (auto& item : vec) {
  //   std::cout << "str: " << item << "\n";
  // }
  return 0;
}


TEST(GlobString, star) {
  glob::glob g("*.pdf");
  ASSERT_TRUE(glob_match("test.pdf", g));
  ASSERT_TRUE(glob_match(".pdf", g));
  ASSERT_FALSE(glob_match("test.txt", g));
  ASSERT_FALSE(glob_match("test.pdff", g));
}

TEST(GlobString, any) {
  glob::glob g("?abc?xy?");
  ASSERT_TRUE(glob_match("qabcqxyq", g));
  ASSERT_TRUE(glob_match("aabcixyp", g));
  ASSERT_FALSE(glob_match("?abc?xy", g));
  ASSERT_FALSE(glob_match("abcxxyx", g));
}

TEST(GlobString, any_star) {
  glob::glob g("?a*.txt");
  ASSERT_TRUE(glob_match("xasefs.txt", g));
  ASSERT_TRUE(glob_match("batest.txt", g));
  ASSERT_FALSE(glob_match("atest.txt", g));
  ASSERT_FALSE(glob_match("batesttxt", g));
}

TEST(GlobString, set1) {
  glob::glob g("*_[0-9].txt");
  ASSERT_TRUE(glob_match("file_1.txt", g));
  ASSERT_TRUE(glob_match("file_5.txt", g));
  ASSERT_TRUE(glob_match("_5.txt", g));
  ASSERT_FALSE(glob_match("file_11.txt", g));
  ASSERT_FALSE(glob_match("file_.txt", g));
}

TEST(GlobString, set2) {
  glob::glob g("*_[a-zA-Z0-9].txt");
  ASSERT_TRUE(glob_match("file_a.txt", g));
  ASSERT_TRUE(glob_match("file_Z.txt", g));
  ASSERT_TRUE(glob_match("_8.txt", g));
  ASSERT_FALSE(glob_match("file_11.txt", g));
  ASSERT_FALSE(glob_match("file_.txt", g));
}

TEST(GlobString, set3) {
  glob::glob g("*_[abc].txt");
  ASSERT_TRUE(glob_match("file_a.txt", g));
  ASSERT_TRUE(glob_match("file_b.txt", g));
  ASSERT_TRUE(glob_match("_c.txt", g));
  ASSERT_FALSE(glob_match("file_d.txt", g));
  ASSERT_FALSE(glob_match("file_z.txt", g));
}

TEST(GlobString, set4) {
  glob::glob g("*_[a-zABC0-9].txt");
  ASSERT_TRUE(glob_match("file_a.txt", g));
  ASSERT_TRUE(glob_match("file_B.txt", g));
  ASSERT_TRUE(glob_match("_3.txt", g));
  ASSERT_FALSE(glob_match("file_D.txt", g));
  ASSERT_FALSE(glob_match("file_E.txt", g));
}

TEST(GlobString, group_plus) {
  glob::glob g("[A-Z]+([a-z0-9]).txt");
  ASSERT_TRUE(glob_match("File1.txt", g));
  ASSERT_TRUE(glob_match("File12.txt", g));
  ASSERT_TRUE(glob_match("F3.txt", g));
  ASSERT_FALSE(glob_match("file.txt", g));
  ASSERT_FALSE(glob_match("F.txt", g));
  ASSERT_FALSE(glob_match("File12.pdf", g));
}

TEST(GlobString, group_star) {
  glob::glob g("*([A-Z])+([a-z0-9]).txt");
  ASSERT_TRUE(glob_match("FILE1.txt", g));
  ASSERT_TRUE(glob_match("file.txt", g));
  ASSERT_TRUE(glob_match("F3.txt", g));
  ASSERT_FALSE(glob_match(".txt", g));
  ASSERT_FALSE(glob_match("_file.txt", g));
  ASSERT_FALSE(glob_match("F.pdf", g));
}