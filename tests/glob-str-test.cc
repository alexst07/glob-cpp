#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "glob-cpp/glob.h"

// Test fixture for common setup
class GlobTestFixture : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

// ============================================================================
// Basic Pattern Tests
// ============================================================================

TEST_F(GlobTestFixture, StarBasic) {
  glob::glob g("*.pdf");
  EXPECT_TRUE(glob::glob_match("test.pdf", g));
  EXPECT_TRUE(glob::glob_match(".pdf", g));
  EXPECT_FALSE(glob::glob_match("test.txt", g));
  EXPECT_FALSE(glob::glob_match("test.pdff", g));
}

TEST_F(GlobTestFixture, StarAtStart) {
  glob::glob g("*test");
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_TRUE(glob::glob_match("atest", g));
  EXPECT_TRUE(glob::glob_match("123test", g));
  EXPECT_FALSE(glob::glob_match("tes", g));
}

TEST_F(GlobTestFixture, StarAtEnd) {
  glob::glob g("test*");
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_TRUE(glob::glob_match("testa", g));
  EXPECT_TRUE(glob::glob_match("test123", g));
  EXPECT_FALSE(glob::glob_match("tes", g));
}

TEST_F(GlobTestFixture, StarOnly) {
  glob::glob g("*");
  EXPECT_TRUE(glob::glob_match("", g));
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_TRUE(glob::glob_match("abc", g));
  EXPECT_TRUE(glob::glob_match("any string", g));
}

TEST_F(GlobTestFixture, AnyBasic) {
  glob::glob g("?abc?xy?");
  EXPECT_TRUE(glob::glob_match("qabcqxyq", g));
  EXPECT_TRUE(glob::glob_match("aabcixyp", g));
  EXPECT_FALSE(glob::glob_match("?abc?xy", g));
  EXPECT_FALSE(glob::glob_match("abcxxyx", g));
}

TEST_F(GlobTestFixture, AnyMultiple) {
  glob::glob g("??");
  EXPECT_TRUE(glob::glob_match("ab", g));
  EXPECT_TRUE(glob::glob_match("12", g));
  EXPECT_FALSE(glob::glob_match("a", g));
  EXPECT_FALSE(glob::glob_match("abc", g));
  
  glob::glob g2("?*?");
  EXPECT_TRUE(glob::glob_match("ab", g2));
  EXPECT_TRUE(glob::glob_match("abc", g2));
  EXPECT_FALSE(glob::glob_match("a", g2));
}

TEST_F(GlobTestFixture, AnyOnly) {
  glob::glob g("?");
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_TRUE(glob::glob_match("1", g));
  EXPECT_FALSE(glob::glob_match("", g));
  EXPECT_FALSE(glob::glob_match("ab", g));
}

TEST_F(GlobTestFixture, AnyStarCombination) {
  glob::glob g("?a*.txt");
  EXPECT_TRUE(glob::glob_match("xasefs.txt", g));
  EXPECT_TRUE(glob::glob_match("batest.txt", g));
  EXPECT_FALSE(glob::glob_match("atest.txt", g));
  EXPECT_FALSE(glob::glob_match("batesttxt", g));
}

TEST_F(GlobTestFixture, EmptyString) {
  glob::glob g("");
  EXPECT_TRUE(glob::glob_match("", g));
  EXPECT_FALSE(glob::glob_match("a", g));
  
  glob::glob g2("*");
  EXPECT_TRUE(glob::glob_match("", g2));
}

TEST_F(GlobTestFixture, SingleCharacter) {
  glob::glob g("a");
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_FALSE(glob::glob_match("", g));
  EXPECT_FALSE(glob::glob_match("ab", g));
  EXPECT_FALSE(glob::glob_match("b", g));
}

TEST_F(GlobTestFixture, LiteralString) {
  glob::glob g("test");
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_FALSE(glob::glob_match("tes", g));
  EXPECT_FALSE(glob::glob_match("testa", g));
}

// ============================================================================
// Character Set Tests
// ============================================================================

TEST_F(GlobTestFixture, SetSingleChar) {
  glob::glob g("[a]");
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_FALSE(glob::glob_match("b", g));
  EXPECT_FALSE(glob::glob_match("", g));
}

TEST_F(GlobTestFixture, SetRange) {
  glob::glob g("*_[0-9].txt");
  EXPECT_TRUE(glob::glob_match("file_1.txt", g));
  EXPECT_TRUE(glob::glob_match("file_5.txt", g));
  EXPECT_TRUE(glob::glob_match("_5.txt", g));
  EXPECT_FALSE(glob::glob_match("file_11.txt", g));
  EXPECT_FALSE(glob::glob_match("file_.txt", g));
}

TEST_F(GlobTestFixture, SetMultipleRanges) {
  glob::glob g("*_[a-zA-Z0-9].txt");
  EXPECT_TRUE(glob::glob_match("file_a.txt", g));
  EXPECT_TRUE(glob::glob_match("file_Z.txt", g));
  EXPECT_TRUE(glob::glob_match("_8.txt", g));
  EXPECT_FALSE(glob::glob_match("file_11.txt", g));
  EXPECT_FALSE(glob::glob_match("file_.txt", g));
}

TEST_F(GlobTestFixture, SetCharList) {
  glob::glob g("*_[abc].txt");
  EXPECT_TRUE(glob::glob_match("file_a.txt", g));
  EXPECT_TRUE(glob::glob_match("file_b.txt", g));
  EXPECT_TRUE(glob::glob_match("_c.txt", g));
  EXPECT_FALSE(glob::glob_match("file_d.txt", g));
  EXPECT_FALSE(glob::glob_match("file_z.txt", g));
}

TEST_F(GlobTestFixture, SetMixed) {
  glob::glob g("*_[a-zABC0-9].txt");
  EXPECT_TRUE(glob::glob_match("file_a.txt", g));
  EXPECT_TRUE(glob::glob_match("file_B.txt", g));
  EXPECT_TRUE(glob::glob_match("_3.txt", g));
  EXPECT_FALSE(glob::glob_match("file_D.txt", g));
  EXPECT_FALSE(glob::glob_match("file_E.txt", g));
}

TEST_F(GlobTestFixture, SetNegative) {
  glob::glob g("[^a-z].txt");
  EXPECT_TRUE(glob::glob_match("A.txt", g));
  EXPECT_TRUE(glob::glob_match("1.txt", g));
  EXPECT_FALSE(glob::glob_match("a.txt", g));
  EXPECT_FALSE(glob::glob_match("z.txt", g));
}

TEST_F(GlobTestFixture, SetNegativeRange) {
  glob::glob g("*[^0-9].txt");
  EXPECT_TRUE(glob::glob_match("filea.txt", g));
  EXPECT_TRUE(glob::glob_match("file_.txt", g));
  EXPECT_FALSE(glob::glob_match("file1.txt", g));
  EXPECT_FALSE(glob::glob_match("file5.txt", g));
}

// ============================================================================
// Group Tests
// ============================================================================

TEST_F(GlobTestFixture, GroupPlus) {
  glob::glob g("[A-Z]+([a-z0-9]).txt");
  EXPECT_TRUE(glob::glob_match("File1.txt", g));
  EXPECT_TRUE(glob::glob_match("File12.txt", g));
  EXPECT_TRUE(glob::glob_match("F3.txt", g));
  EXPECT_FALSE(glob::glob_match("file.txt", g));
  EXPECT_FALSE(glob::glob_match("F.txt", g));
  EXPECT_FALSE(glob::glob_match("File12.pdf", g));
}

TEST_F(GlobTestFixture, GroupStar) {
  glob::glob g("*([A-Z])+([a-z0-9]).txt");
  EXPECT_TRUE(glob::glob_match("FILE1.txt", g));
  EXPECT_TRUE(glob::glob_match("file.txt", g));
  EXPECT_TRUE(glob::glob_match("F3.txt", g));
  EXPECT_FALSE(glob::glob_match(".txt", g));
  EXPECT_FALSE(glob::glob_match("_file.txt", g));
  EXPECT_FALSE(glob::glob_match("F.pdf", g));
}

TEST_F(GlobTestFixture, GroupAny) {
  glob::glob g("*([A-Z])?([a-z0-9]).txt");
  EXPECT_TRUE(glob::glob_match("FILE1.txt", g));
  EXPECT_TRUE(glob::glob_match("FILE.txt", g));
  EXPECT_TRUE(glob::glob_match("F3.txt", g));
  EXPECT_TRUE(glob::glob_match(".txt", g));
  EXPECT_FALSE(glob::glob_match("FILE12.txt", g));
  EXPECT_FALSE(glob::glob_match("FF.pdf", g));
}

TEST_F(GlobTestFixture, GroupAt) {
  glob::glob g("*([A-Z])@([a-z0-9]).txt");
  EXPECT_TRUE(glob::glob_match("FILE1.txt", g));
  EXPECT_TRUE(glob::glob_match("FILEx.txt", g));
  EXPECT_TRUE(glob::glob_match("F3.txt", g));
  EXPECT_FALSE(glob::glob_match(".txt", g));
  EXPECT_FALSE(glob::glob_match("FILE.txt", g));
  EXPECT_FALSE(glob::glob_match("FF.pdf", g));
}

TEST_F(GlobTestFixture, GroupNeg) {
  glob::glob g("!([a-z]).txt");
  EXPECT_TRUE(glob::glob_match("A.txt", g));
  EXPECT_TRUE(glob::glob_match("1.txt", g));
  EXPECT_FALSE(glob::glob_match("a.txt", g));
}

TEST_F(GlobTestFixture, GroupUnion) {
  glob::glob g("*([a-zA-Z])*([0-9]).(txt|pdf)");
  EXPECT_TRUE(glob::glob_match("FILE1.txt", g));
  EXPECT_TRUE(glob::glob_match("FILE1.pdf", g));
  EXPECT_TRUE(glob::glob_match("FILE.pdf", g));
  EXPECT_TRUE(glob::glob_match("F3.txt", g));
  EXPECT_TRUE(glob::glob_match(".txt", g));
  EXPECT_FALSE(glob::glob_match("FILE.jpg", g));
  EXPECT_FALSE(glob::glob_match("FF.sdf", g));
}

TEST_F(GlobTestFixture, GroupMultipleUnions) {
  glob::glob g("(a|b|c|d)");
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_TRUE(glob::glob_match("b", g));
  EXPECT_TRUE(glob::glob_match("c", g));
  EXPECT_TRUE(glob::glob_match("d", g));
  EXPECT_FALSE(glob::glob_match("e", g));
}

TEST_F(GlobTestFixture, GroupNested) {
  glob::glob g("*((a|b)|(c|d))");
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_TRUE(glob::glob_match("b", g));
  EXPECT_TRUE(glob::glob_match("c", g));
  EXPECT_TRUE(glob::glob_match("d", g));
  EXPECT_TRUE(glob::glob_match("", g));
  EXPECT_FALSE(glob::glob_match("e", g));
}

TEST_F(GlobTestFixture, GroupComplex) {
  glob::glob g("*([a-z])+([0-9]).(txt|pdf)");
  EXPECT_TRUE(glob::glob_match("file1.txt", g));
  EXPECT_TRUE(glob::glob_match("file123.pdf", g));
  EXPECT_FALSE(glob::glob_match("file.txt", g));
  EXPECT_FALSE(glob::glob_match("file.jpg", g));
}

// ============================================================================
// Escape Sequence Tests
// ============================================================================

TEST_F(GlobTestFixture, EscapeStar) {
  glob::glob g("\\*");
  EXPECT_TRUE(glob::glob_match("*", g));
  EXPECT_FALSE(glob::glob_match("a", g));
  EXPECT_FALSE(glob::glob_match("", g));
}

TEST_F(GlobTestFixture, EscapeQuestion) {
  glob::glob g("\\?");
  EXPECT_TRUE(glob::glob_match("?", g));
  EXPECT_FALSE(glob::glob_match("a", g));
}

TEST_F(GlobTestFixture, EscapePlus) {
  glob::glob g("\\+");
  EXPECT_TRUE(glob::glob_match("+", g));
  EXPECT_FALSE(glob::glob_match("a", g));
}

TEST_F(GlobTestFixture, EscapeParen) {
  glob::glob g("\\(");
  EXPECT_TRUE(glob::glob_match("(", g));
  EXPECT_FALSE(glob::glob_match("a", g));
  
  glob::glob g2("\\)");
  EXPECT_TRUE(glob::glob_match(")", g2));
}

TEST_F(GlobTestFixture, EscapeBracket) {
  glob::glob g("\\[");
  EXPECT_TRUE(glob::glob_match("[", g));
  
  glob::glob g2("\\]");
  EXPECT_TRUE(glob::glob_match("]", g2));
}

TEST_F(GlobTestFixture, EscapePipe) {
  glob::glob g("\\|");
  EXPECT_TRUE(glob::glob_match("|", g));
}

TEST_F(GlobTestFixture, EscapeExclamation) {
  glob::glob g("\\!");
  EXPECT_TRUE(glob::glob_match("!", g));
}

TEST_F(GlobTestFixture, EscapeAt) {
  glob::glob g("\\@");
  EXPECT_TRUE(glob::glob_match("@", g));
}

TEST_F(GlobTestFixture, EscapeBackslash) {
  glob::glob g("\\\\");
  EXPECT_TRUE(glob::glob_match("\\", g));
}

TEST_F(GlobTestFixture, EscapeInSet) {
  glob::glob g("[\\*\\?]");
  EXPECT_TRUE(glob::glob_match("*", g));
  EXPECT_TRUE(glob::glob_match("?", g));
  EXPECT_FALSE(glob::glob_match("a", g));
}

TEST_F(GlobTestFixture, EscapeInPattern) {
  glob::glob g("test\\*.txt");
  EXPECT_TRUE(glob::glob_match("test*.txt", g));
  EXPECT_FALSE(glob::glob_match("testa.txt", g));
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(GlobTestFixture, ErrorUnclosedBracket) {
  EXPECT_THROW({
    glob::glob g("[abc");
  }, glob::Error);
}

TEST_F(GlobTestFixture, ErrorUnclosedNegativeBracket) {
  EXPECT_THROW({
    glob::glob g("[^abc");
  }, glob::Error);
}

TEST_F(GlobTestFixture, ErrorUnclosedGroup) {
  EXPECT_THROW({
    glob::glob g("(abc");
  }, glob::Error);
}

TEST_F(GlobTestFixture, ErrorUnclosedStarGroup) {
  EXPECT_THROW({
    glob::glob g("*(abc");
  }, glob::Error);
}

TEST_F(GlobTestFixture, ErrorInvalidEscape) {
  EXPECT_THROW({
    glob::glob g("\\");
  }, glob::Error);
}

TEST_F(GlobTestFixture, ErrorInvalidRange) {
  // These might throw or might be handled differently - test actual behavior
  EXPECT_THROW({
    glob::glob g("[a-]");
  }, glob::Error);
}

TEST_F(GlobTestFixture, ErrorInvalidRangeStart) {
  EXPECT_THROW({
    glob::glob g("[-a]");
  }, glob::Error);
}

// ============================================================================
// MatchResults Tests
// ============================================================================

TEST_F(GlobTestFixture, MatchResultsStar) {
  glob::glob g("test*.txt");
  glob::cmatch m;
  EXPECT_TRUE(glob::glob_match("test123.txt", m, g));
  EXPECT_FALSE(m.empty());
}

TEST_F(GlobTestFixture, MatchResultsQuestion) {
  glob::glob g("test?.txt");
  glob::cmatch m;
  EXPECT_TRUE(glob::glob_match("test1.txt", m, g));
  EXPECT_FALSE(m.empty());
}

TEST_F(GlobTestFixture, MatchResultsSet) {
  glob::glob g("test[0-9].txt");
  glob::cmatch m;
  EXPECT_TRUE(glob::glob_match("test5.txt", m, g));
  EXPECT_FALSE(m.empty());
}

TEST_F(GlobTestFixture, MatchResultsGroup) {
  glob::glob g("*(test)file.txt");
  glob::cmatch m;
  EXPECT_TRUE(glob::glob_match("testtestfile.txt", m, g));
  EXPECT_FALSE(m.empty());
}

TEST_F(GlobTestFixture, MatchResultsMultiple) {
  glob::glob g("*test*file*");
  glob::cmatch m;
  EXPECT_TRUE(glob::glob_match("atestbfilec", m, g));
  // Should capture multiple star matches
  EXPECT_FALSE(m.empty());
}

TEST_F(GlobTestFixture, MatchResultsEmpty) {
  glob::glob g("test");
  glob::cmatch m;
  EXPECT_TRUE(glob::glob_match("test", m, g));
  // No wildcards, so results might be empty
}

// ============================================================================
// Boundary Condition Tests
// ============================================================================

TEST_F(GlobTestFixture, LongPattern) {
  std::string long_pattern(1000, 'a');
  long_pattern += "*";
  glob::glob g(long_pattern);
  EXPECT_TRUE(glob::glob_match(long_pattern.substr(0, 1000) + "test", g));
}

TEST_F(GlobTestFixture, LongString) {
  std::string long_string(10000, 'a');
  glob::glob g("*");
  EXPECT_TRUE(glob::glob_match(long_string, g));
}

TEST_F(GlobTestFixture, ComplexPattern) {
  glob::glob g("*([a-z])+([0-9])*(.txt|.pdf|.jpg)");
  EXPECT_TRUE(glob::glob_match("file123.txt", g));
  EXPECT_TRUE(glob::glob_match("abc456.pdf", g));
  EXPECT_TRUE(glob::glob_match("123.txt", g)); // Shell: *([a-z]) allows zero letters
}

TEST_F(GlobTestFixture, ZeroLengthMatch) {
  glob::glob g("*(test)");
  EXPECT_TRUE(glob::glob_match("", g));
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_TRUE(glob::glob_match("testtest", g));
}

// ============================================================================
// Wide Character Tests
// ============================================================================

TEST_F(GlobTestFixture, WideCharBasic) {
  glob::wglob g(L"*.txt");
  EXPECT_TRUE(glob::glob_match(L"test.txt", g));
  EXPECT_TRUE(glob::glob_match(L"file.txt", g));
  EXPECT_FALSE(glob::glob_match(L"test.pdf", g));
}

TEST_F(GlobTestFixture, WideCharStar) {
  glob::wglob g(L"test*");
  EXPECT_TRUE(glob::glob_match(L"test", g));
  EXPECT_TRUE(glob::glob_match(L"test123", g));
  EXPECT_FALSE(glob::glob_match(L"tes", g));
}

TEST_F(GlobTestFixture, WideCharSet) {
  glob::wglob g(L"[a-z]*");
  EXPECT_TRUE(glob::glob_match(L"test", g));
  EXPECT_TRUE(glob::glob_match(L"file", g));
  EXPECT_FALSE(glob::glob_match(L"TEST", g));
}

TEST_F(GlobTestFixture, WideCharGroup) {
  glob::wglob g(L"*(test|file)");
  EXPECT_TRUE(glob::glob_match(L"test", g));
  EXPECT_TRUE(glob::glob_match(L"file", g));
  EXPECT_TRUE(glob::glob_match(L"testfile", g));
  EXPECT_FALSE(glob::glob_match(L"other", g));
}

// ============================================================================
// Parameterized Tests for Similar Cases
// ============================================================================

class GlobPatternTest : public ::testing::TestWithParam<std::tuple<std::string, std::string, bool>> {};

TEST_P(GlobPatternTest, PatternMatching) {
  std::string pattern = std::get<0>(GetParam());
  std::string input = std::get<1>(GetParam());
  bool expected = std::get<2>(GetParam());
  glob::glob g(pattern);
  EXPECT_EQ(expected, glob::glob_match(input, g));
}

INSTANTIATE_TEST_CASE_P(
    BasicPatterns,
    GlobPatternTest,
    ::testing::Values(
        std::make_tuple("*.txt", "file.txt", true),
        std::make_tuple("*.txt", "file.pdf", false),
        std::make_tuple("test?", "test1", true),
        std::make_tuple("test?", "test", false),
        std::make_tuple("test?", "test12", false),
        std::make_tuple("[a-z]*", "test", true),
        std::make_tuple("[a-z]*", "TEST", false)
    )
);

// ============================================================================
// Special Character Tests
// ============================================================================

TEST_F(GlobTestFixture, SpecialCharsInPattern) {
  // Test that special characters need escaping
  glob::glob g("test\\*.txt");
  EXPECT_TRUE(glob::glob_match("test*.txt", g));
  EXPECT_FALSE(glob::glob_match("testa.txt", g));
}

TEST_F(GlobTestFixture, DotInPattern) {
  glob::glob g("*.txt");
  EXPECT_TRUE(glob::glob_match("file.txt", g));
  EXPECT_FALSE(glob::glob_match("filetxt", g));
}

TEST_F(GlobTestFixture, UnderscoreInPattern) {
  glob::glob g("test_file.txt");
  EXPECT_TRUE(glob::glob_match("test_file.txt", g));
  EXPECT_FALSE(glob::glob_match("testfile.txt", g));
}

TEST_F(GlobTestFixture, HyphenInPattern) {
  glob::glob g("test-file.txt");
  EXPECT_TRUE(glob::glob_match("test-file.txt", g));
  EXPECT_FALSE(glob::glob_match("testfile.txt", g));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(GlobTestFixture, PatternStartsWithWildcard) {
  glob::glob g("*test");
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_TRUE(glob::glob_match("atest", g));
  EXPECT_FALSE(glob::glob_match("tes", g));
}

TEST_F(GlobTestFixture, PatternEndsWithWildcard) {
  glob::glob g("test*");
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_TRUE(glob::glob_match("testa", g));
  EXPECT_FALSE(glob::glob_match("tes", g));
}

TEST_F(GlobTestFixture, PatternAllWildcards) {
  glob::glob g("***");
  EXPECT_TRUE(glob::glob_match("", g));
  EXPECT_TRUE(glob::glob_match("a", g));
  EXPECT_TRUE(glob::glob_match("abc", g));
}

TEST_F(GlobTestFixture, ConsecutiveWildcards) {
  glob::glob g("test**file");
  EXPECT_TRUE(glob::glob_match("testfile", g));
  EXPECT_TRUE(glob::glob_match("test123file", g));
}

TEST_F(GlobTestFixture, QuestionStarCombination) {
  glob::glob g("test?*file");
  EXPECT_TRUE(glob::glob_match("test1file", g));
  EXPECT_TRUE(glob::glob_match("test123file", g));
  EXPECT_FALSE(glob::glob_match("testfile", g));
}

TEST_F(GlobTestFixture, DoubleAsteriskPattern) {
  glob::glob g("https://**.google.com");
  EXPECT_TRUE(glob::glob_match("https://foo.bar.google.com", g));
  EXPECT_FALSE(glob::glob_match("https://google.com", g)); // Shell: ** in middle requires at least one character
  EXPECT_TRUE(glob::glob_match("https://a.google.com", g));
  EXPECT_TRUE(glob::glob_match("https://a.b.c.google.com", g));
}

// ============================================================================
// Brace Expansion Tests
// ============================================================================

TEST_F(GlobTestFixture, BraceExpansionBasic) {
  glob::glob g("*.{h,hpp}");
  EXPECT_TRUE(glob::glob_match("file.h", g));
  EXPECT_TRUE(glob::glob_match("test.hpp", g));
  EXPECT_FALSE(glob::glob_match("file.c", g));
  EXPECT_FALSE(glob::glob_match("file.hh", g));
}

TEST_F(GlobTestFixture, BraceExpansionMultipleItems) {
  glob::glob g("*.{h,hpp,c,cpp}");
  EXPECT_TRUE(glob::glob_match("file.h", g));
  EXPECT_TRUE(glob::glob_match("file.hpp", g));
  EXPECT_TRUE(glob::glob_match("file.c", g));
  EXPECT_TRUE(glob::glob_match("file.cpp", g));
  EXPECT_FALSE(glob::glob_match("file.txt", g));
  EXPECT_FALSE(glob::glob_match("file.hh", g));
}

TEST_F(GlobTestFixture, BraceExpansionWithPrefix) {
  glob::glob g("test.{txt,md}");
  EXPECT_TRUE(glob::glob_match("test.txt", g));
  EXPECT_TRUE(glob::glob_match("test.md", g));
  EXPECT_FALSE(glob::glob_match("test.pdf", g));
  EXPECT_FALSE(glob::glob_match("atest.txt", g));
}

TEST_F(GlobTestFixture, BraceExpansionAtStart) {
  glob::glob g("{a,b}*.txt");
  EXPECT_TRUE(glob::glob_match("a.txt", g));
  EXPECT_TRUE(glob::glob_match("b.txt", g));
  EXPECT_TRUE(glob::glob_match("a123.txt", g));
  EXPECT_TRUE(glob::glob_match("bfile.txt", g));
  EXPECT_FALSE(glob::glob_match("c.txt", g));
  EXPECT_FALSE(glob::glob_match("ab.txt", g));
}

TEST_F(GlobTestFixture, BraceExpansionSingleItem) {
  glob::glob g("*.{h}");
  EXPECT_TRUE(glob::glob_match("file.h", g));
  EXPECT_FALSE(glob::glob_match("file.hpp", g));
  EXPECT_FALSE(glob::glob_match("file.c", g));
}

TEST_F(GlobTestFixture, BraceExpansionWithWildcards) {
  glob::glob g("test*.{txt,pdf}");
  EXPECT_TRUE(glob::glob_match("test.txt", g));
  EXPECT_TRUE(glob::glob_match("test123.pdf", g));
  EXPECT_TRUE(glob::glob_match("test_file.txt", g));
  EXPECT_FALSE(glob::glob_match("test.jpg", g));
}

TEST_F(GlobTestFixture, BraceExpansionNested) {
  glob::glob g("*.{h{pp,xx},c}");
  EXPECT_TRUE(glob::glob_match("file.hpp", g));
  EXPECT_TRUE(glob::glob_match("file.hxx", g));
  EXPECT_TRUE(glob::glob_match("file.c", g));
  EXPECT_FALSE(glob::glob_match("file.h", g));
  EXPECT_FALSE(glob::glob_match("file.hppp", g));
}

TEST_F(GlobTestFixture, BraceExpansionNestedComplex) {
  glob::glob g("{a,b{1,2}}*.txt");
  EXPECT_TRUE(glob::glob_match("a.txt", g));
  EXPECT_TRUE(glob::glob_match("b1.txt", g));
  EXPECT_TRUE(glob::glob_match("b2.txt", g));
  EXPECT_TRUE(glob::glob_match("afile.txt", g));
  EXPECT_TRUE(glob::glob_match("b1test.txt", g));
  EXPECT_FALSE(glob::glob_match("b.txt", g));
}

TEST_F(GlobTestFixture, BraceExpansionEmptyBraces) {
  // Empty braces should match empty string
  glob::glob g("test{}");
  EXPECT_TRUE(glob::glob_match("test", g));
  EXPECT_FALSE(glob::glob_match("testx", g));
}

TEST_F(GlobTestFixture, BraceExpansionTrailingComma) {
  glob::glob g("*.{h,}");
  EXPECT_TRUE(glob::glob_match("file.h", g));
  EXPECT_TRUE(glob::glob_match("file.", g)); // trailing comma adds empty item
  EXPECT_FALSE(glob::glob_match("file.c", g));
}

TEST_F(GlobTestFixture, BraceExpansionLeadingComma) {
  glob::glob g("*.{,h}");
  EXPECT_TRUE(glob::glob_match("file.", g)); // leading comma adds empty item
  EXPECT_TRUE(glob::glob_match("file.h", g));
  EXPECT_FALSE(glob::glob_match("file.c", g));
}

TEST_F(GlobTestFixture, BraceExpansionWithSets) {
  glob::glob g("file[0-9].{txt,pdf}");
  EXPECT_TRUE(glob::glob_match("file1.txt", g));
  EXPECT_TRUE(glob::glob_match("file5.pdf", g));
  EXPECT_FALSE(glob::glob_match("filea.txt", g));
  EXPECT_FALSE(glob::glob_match("file1.jpg", g));
}

TEST_F(GlobTestFixture, BraceExpansionErrorUnclosed) {
  EXPECT_THROW({
    glob::glob g("*.{h,hpp");
  }, glob::Error);
}

TEST_F(GlobTestFixture, BraceExpansionEscaped) {
  glob::glob g("\\{test\\}");
  EXPECT_TRUE(glob::glob_match("{test}", g));
  EXPECT_FALSE(glob::glob_match("test", g));
}

TEST_F(GlobTestFixture, BraceExpansionComplex) {
  glob::glob g("prefix*{a,b}*suffix.{ext1,ext2}");
  // Pattern expands to: prefix*a*suffix.ext1, prefix*a*suffix.ext2,
  //                     prefix*b*suffix.ext1, prefix*b*suffix.ext2
  // All expanded patterns require the literal "suffix" text
  EXPECT_FALSE(glob::glob_match("prefixxaext1", g));  // Missing "suffix"
  EXPECT_FALSE(glob::glob_match("prefixxbext2", g));  // Missing "suffix"
  EXPECT_TRUE(glob::glob_match("prefix123a456suffix.ext1", g));
  EXPECT_TRUE(glob::glob_match("prefixxa456suffix.ext1", g));  // Has "suffix"
  EXPECT_TRUE(glob::glob_match("prefixxbsuffix.ext2", g));  // Has "suffix"
  EXPECT_FALSE(glob::glob_match("prefixxsuffix.ext3", g));
}
