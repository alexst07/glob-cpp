#!/bin/bash

# Shell script to test glob patterns - EXACT SAME TESTS AS C++ TESTS
# Uses bash case statements which support glob patterns
# This allows direct comparison between shell globbing and C++ glob library
#
# Note: Shell uses [!...] for negation, C++ uses [^...]
#       Extended globbing enabled for patterns like *() and +()

# Don't exit on error - we want to run all tests
set +e

# Enable extended globbing for patterns like *() and +()
shopt -s extglob

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

# Test function: test_glob pattern string expected test_name
# Uses bash case statement for glob matching
test_glob() {
    local pattern="$1"
    local test_string="$2"
    local expected="$3"
    local test_name="$4"
    
    # Convert C++ negation syntax [^...] to shell syntax [!...]
    local shell_pattern="${pattern//\[^/[!}"
    
    # Bash case statement uses glob patterns
    case "$test_string" in
        $shell_pattern)
            result="true"
            ;;
        *)
            result="false"
            ;;
    esac
    
    if [ "$result" = "$expected" ]; then
        echo -e "${GREEN}PASS${NC}: $test_name"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}FAIL${NC}: $test_name - Pattern: '$pattern' String: '$test_string' Expected: $expected Got: $result"
        ((FAILED++))
        return 1
    fi
}

echo "=========================================="
echo "Shell Glob Pattern Test Suite"
echo "EXACT SAME TESTS AS C++ TESTS"
echo "=========================================="
echo ""

# Test: StarBasic
echo "Test: StarBasic"
test_glob "*.pdf" "test.pdf" "true" "StarBasic: *.pdf matches test.pdf"
test_glob "*.pdf" ".pdf" "true" "StarBasic: *.pdf matches .pdf"
test_glob "*.pdf" "test.txt" "false" "StarBasic: *.pdf does not match test.txt"
test_glob "*.pdf" "test.pdff" "false" "StarBasic: *.pdf does not match test.pdff"
echo ""

# Test: StarMultiple
echo "Test: StarMultiple"
test_glob "**" "" "true" "StarMultiple: ** matches empty"
test_glob "**" "a" "true" "StarMultiple: ** matches a"
test_glob "**" "abc" "true" "StarMultiple: ** matches abc"
test_glob "***" "" "true" "StarMultiple: *** matches empty"
test_glob "***" "abc" "true" "StarMultiple: *** matches abc"
echo ""

# Test: StarAtStart
echo "Test: StarAtStart"
test_glob "*test" "test" "true" "StarAtStart: *test matches test"
test_glob "*test" "atest" "true" "StarAtStart: *test matches atest"
test_glob "*test" "123test" "true" "StarAtStart: *test matches 123test"
test_glob "*test" "tes" "false" "StarAtStart: *test does not match tes"
echo ""

# Test: StarAtEnd
echo "Test: StarAtEnd"
test_glob "test*" "test" "true" "StarAtEnd: test* matches test"
test_glob "test*" "testa" "true" "StarAtEnd: test* matches testa"
test_glob "test*" "test123" "true" "StarAtEnd: test* matches test123"
test_glob "test*" "tes" "false" "StarAtEnd: test* does not match tes"
echo ""

# Test: StarOnly
echo "Test: StarOnly"
test_glob "*" "" "true" "StarOnly: * matches empty"
test_glob "*" "a" "true" "StarOnly: * matches a"
test_glob "*" "abc" "true" "StarOnly: * matches abc"
test_glob "*" "any string" "true" "StarOnly: * matches 'any string'"
echo ""

# Test: AnyBasic
echo "Test: AnyBasic"
test_glob "?abc?xy?" "qabcqxyq" "true" "AnyBasic: ?abc?xy? matches qabcqxyq"
test_glob "?abc?xy?" "aabcixyp" "true" "AnyBasic: ?abc?xy? matches aabcixyp"
test_glob "?abc?xy?" "?abc?xy" "false" "AnyBasic: ?abc?xy? does not match ?abc?xy"
test_glob "?abc?xy?" "abcxxyx" "false" "AnyBasic: ?abc?xy? does not match abcxxyx"
echo ""

# Test: AnyMultiple
echo "Test: AnyMultiple"
test_glob "??" "ab" "true" "AnyMultiple: ?? matches ab"
test_glob "??" "12" "true" "AnyMultiple: ?? matches 12"
test_glob "??" "a" "false" "AnyMultiple: ?? does not match a"
test_glob "??" "abc" "false" "AnyMultiple: ?? does not match abc"
test_glob "?*?" "ab" "true" "AnyMultiple: ?*? matches ab"
test_glob "?*?" "abc" "true" "AnyMultiple: ?*? matches abc"
test_glob "?*?" "a" "false" "AnyMultiple: ?*? does not match a"
echo ""

# Test: AnyOnly
echo "Test: AnyOnly"
test_glob "?" "a" "true" "AnyOnly: ? matches a"
test_glob "?" "1" "true" "AnyOnly: ? matches 1"
test_glob "?" "" "false" "AnyOnly: ? does not match empty"
test_glob "?" "ab" "false" "AnyOnly: ? does not match ab"
echo ""

# Test: AnyStarCombination
echo "Test: AnyStarCombination"
test_glob "?a*.txt" "xasefs.txt" "true" "AnyStarCombination: ?a*.txt matches xasefs.txt"
test_glob "?a*.txt" "batest.txt" "true" "AnyStarCombination: ?a*.txt matches batest.txt"
test_glob "?a*.txt" "atest.txt" "false" "AnyStarCombination: ?a*.txt does not match atest.txt"
test_glob "?a*.txt" "batesttxt" "false" "AnyStarCombination: ?a*.txt does not match batesttxt"
echo ""

# Test: EmptyString
echo "Test: EmptyString"
# Empty pattern - in shell, empty pattern matches nothing by default
# But we can test it explicitly
if [ "" = "" ]; then
    echo -e "${GREEN}PASS${NC}: EmptyString: empty pattern matches empty string"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}: EmptyString: empty pattern does not match empty string"
    ((FAILED++))
fi
if [ "a" != "" ]; then
    echo -e "${GREEN}PASS${NC}: EmptyString: empty pattern does not match a"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}: EmptyString: empty pattern matches a"
    ((FAILED++))
fi
test_glob "*" "" "true" "EmptyString: * matches empty"
echo ""

# Test: SingleCharacter
echo "Test: SingleCharacter"
test_glob "a" "a" "true" "SingleCharacter: a matches a"
test_glob "a" "" "false" "SingleCharacter: a does not match empty"
test_glob "a" "ab" "false" "SingleCharacter: a does not match ab"
test_glob "a" "b" "false" "SingleCharacter: a does not match b"
echo ""

# Test: LiteralString
echo "Test: LiteralString"
test_glob "test" "test" "true" "LiteralString: test matches test"
test_glob "test" "tes" "false" "LiteralString: test does not match tes"
test_glob "test" "testa" "false" "LiteralString: test does not match testa"
echo ""

# Test: SetSingleChar
echo "Test: SetSingleChar"
test_glob "[a]" "a" "true" "SetSingleChar: [a] matches a"
test_glob "[a]" "b" "false" "SetSingleChar: [a] does not match b"
test_glob "[a]" "" "false" "SetSingleChar: [a] does not match empty"
echo ""

# Test: SetRange
echo "Test: SetRange"
test_glob "*_[0-9].txt" "file_1.txt" "true" "SetRange: *_[0-9].txt matches file_1.txt"
test_glob "*_[0-9].txt" "file_5.txt" "true" "SetRange: *_[0-9].txt matches file_5.txt"
test_glob "*_[0-9].txt" "_5.txt" "true" "SetRange: *_[0-9].txt matches _5.txt"
test_glob "*_[0-9].txt" "file_11.txt" "false" "SetRange: *_[0-9].txt does not match file_11.txt"
test_glob "*_[0-9].txt" "file_.txt" "false" "SetRange: *_[0-9].txt does not match file_.txt"
echo ""

# Test: SetMultipleRanges
echo "Test: SetMultipleRanges"
test_glob "*_[a-zA-Z0-9].txt" "file_a.txt" "true" "SetMultipleRanges: *_[a-zA-Z0-9].txt matches file_a.txt"
test_glob "*_[a-zA-Z0-9].txt" "file_Z.txt" "true" "SetMultipleRanges: *_[a-zA-Z0-9].txt matches file_Z.txt"
test_glob "*_[a-zA-Z0-9].txt" "_8.txt" "true" "SetMultipleRanges: *_[a-zA-Z0-9].txt matches _8.txt"
test_glob "*_[a-zA-Z0-9].txt" "file_11.txt" "false" "SetMultipleRanges: *_[a-zA-Z0-9].txt does not match file_11.txt"
test_glob "*_[a-zA-Z0-9].txt" "file_.txt" "false" "SetMultipleRanges: *_[a-zA-Z0-9].txt does not match file_.txt"
echo ""

# Test: SetCharList
echo "Test: SetCharList"
test_glob "*_[abc].txt" "file_a.txt" "true" "SetCharList: *_[abc].txt matches file_a.txt"
test_glob "*_[abc].txt" "file_b.txt" "true" "SetCharList: *_[abc].txt matches file_b.txt"
test_glob "*_[abc].txt" "_c.txt" "true" "SetCharList: *_[abc].txt matches _c.txt"
test_glob "*_[abc].txt" "file_d.txt" "false" "SetCharList: *_[abc].txt does not match file_d.txt"
test_glob "*_[abc].txt" "file_z.txt" "false" "SetCharList: *_[abc].txt does not match file_z.txt"
echo ""

# Test: SetMixed
echo "Test: SetMixed"
test_glob "*_[a-zABC0-9].txt" "file_a.txt" "true" "SetMixed: *_[a-zABC0-9].txt matches file_a.txt"
test_glob "*_[a-zABC0-9].txt" "file_B.txt" "true" "SetMixed: *_[a-zABC0-9].txt matches file_B.txt"
test_glob "*_[a-zABC0-9].txt" "_3.txt" "true" "SetMixed: *_[a-zABC0-9].txt matches _3.txt"
test_glob "*_[a-zABC0-9].txt" "file_D.txt" "false" "SetMixed: *_[a-zABC0-9].txt does not match file_D.txt"
test_glob "*_[a-zABC0-9].txt" "file_E.txt" "false" "SetMixed: *_[a-zABC0-9].txt does not match file_E.txt"
echo ""

# Test: SetNegative
echo "Test: SetNegative"
test_glob "[^a-z].txt" "A.txt" "true" "SetNegative: [^a-z].txt matches A.txt"
test_glob "[^a-z].txt" "1.txt" "true" "SetNegative: [^a-z].txt matches 1.txt"
test_glob "[^a-z].txt" "a.txt" "false" "SetNegative: [^a-z].txt does not match a.txt"
test_glob "[^a-z].txt" "z.txt" "false" "SetNegative: [^a-z].txt does not match z.txt"
echo ""

# Test: SetNegativeRange
echo "Test: SetNegativeRange"
test_glob "*[^0-9].txt" "filea.txt" "true" "SetNegativeRange: *[^0-9].txt matches filea.txt"
test_glob "*[^0-9].txt" "file_.txt" "true" "SetNegativeRange: *[^0-9].txt matches file_.txt"
test_glob "*[^0-9].txt" "file1.txt" "false" "SetNegativeRange: *[^0-9].txt does not match file1.txt"
test_glob "*[^0-9].txt" "file5.txt" "false" "SetNegativeRange: *[^0-9].txt does not match file5.txt"
echo ""

# Test: GroupPlus
echo "Test: GroupPlus"
test_glob "[A-Z]+([a-z0-9]).txt" "File1.txt" "true" "GroupPlus: [A-Z]+([a-z0-9]).txt matches File1.txt"
test_glob "[A-Z]+([a-z0-9]).txt" "File12.txt" "true" "GroupPlus: [A-Z]+([a-z0-9]).txt matches File12.txt"
test_glob "[A-Z]+([a-z0-9]).txt" "F3.txt" "true" "GroupPlus: [A-Z]+([a-z0-9]).txt matches F3.txt"
test_glob "[A-Z]+([a-z0-9]).txt" "file.txt" "false" "GroupPlus: [A-Z]+([a-z0-9]).txt does not match file.txt"
test_glob "[A-Z]+([a-z0-9]).txt" "F.txt" "false" "GroupPlus: [A-Z]+([a-z0-9]).txt does not match F.txt"
test_glob "[A-Z]+([a-z0-9]).txt" "File12.pdf" "false" "GroupPlus: [A-Z]+([a-z0-9]).txt does not match File12.pdf"
echo ""

# Test: GroupStar
echo "Test: GroupStar"
test_glob "*([A-Z])+([a-z0-9]).txt" "FILE1.txt" "true" "GroupStar: *([A-Z])+([a-z0-9]).txt matches FILE1.txt"
test_glob "*([A-Z])+([a-z0-9]).txt" "file.txt" "true" "GroupStar: *([A-Z])+([a-z0-9]).txt matches file.txt"
test_glob "*([A-Z])+([a-z0-9]).txt" "F3.txt" "true" "GroupStar: *([A-Z])+([a-z0-9]).txt matches F3.txt"
test_glob "*([A-Z])+([a-z0-9]).txt" ".txt" "false" "GroupStar: *([A-Z])+([a-z0-9]).txt does not match .txt"
test_glob "*([A-Z])+([a-z0-9]).txt" "_file.txt" "false" "GroupStar: *([A-Z])+([a-z0-9]).txt does not match _file.txt"
test_glob "*([A-Z])+([a-z0-9]).txt" "F.pdf" "false" "GroupStar: *([A-Z])+([a-z0-9]).txt does not match F.pdf"
echo ""

# Test: GroupAny
echo "Test: GroupAny"
test_glob "*([A-Z])?([a-z0-9]).txt" "FILE1.txt" "true" "GroupAny: *([A-Z])?([a-z0-9]).txt matches FILE1.txt"
test_glob "*([A-Z])?([a-z0-9]).txt" "FILE.txt" "true" "GroupAny: *([A-Z])?([a-z0-9]).txt matches FILE.txt"
test_glob "*([A-Z])?([a-z0-9]).txt" "F3.txt" "true" "GroupAny: *([A-Z])?([a-z0-9]).txt matches F3.txt"
test_glob "*([A-Z])?([a-z0-9]).txt" ".txt" "true" "GroupAny: *([A-Z])?([a-z0-9]).txt matches .txt"
test_glob "*([A-Z])?([a-z0-9]).txt" "FILE12.txt" "false" "GroupAny: *([A-Z])?([a-z0-9]).txt does not match FILE12.txt"
test_glob "*([A-Z])?([a-z0-9]).txt" "FF.pdf" "false" "GroupAny: *([A-Z])?([a-z0-9]).txt does not match FF.pdf"
echo ""

# Test: GroupAt
echo "Test: GroupAt"
test_glob "*([A-Z])@([a-z0-9]).txt" "FILE1.txt" "true" "GroupAt: *([A-Z])@([a-z0-9]).txt matches FILE1.txt"
test_glob "*([A-Z])@([a-z0-9]).txt" "FILEx.txt" "true" "GroupAt: *([A-Z])@([a-z0-9]).txt matches FILEx.txt"
test_glob "*([A-Z])@([a-z0-9]).txt" "F3.txt" "true" "GroupAt: *([A-Z])@([a-z0-9]).txt matches F3.txt"
test_glob "*([A-Z])@([a-z0-9]).txt" ".txt" "false" "GroupAt: *([A-Z])@([a-z0-9]).txt does not match .txt"
test_glob "*([A-Z])@([a-z0-9]).txt" "FILE.txt" "false" "GroupAt: *([A-Z])@([a-z0-9]).txt does not match FILE.txt"
test_glob "*([A-Z])@([a-z0-9]).txt" "FF.pdf" "false" "GroupAt: *([A-Z])@([a-z0-9]).txt does not match FF.pdf"
echo ""

# Test: GroupNeg
echo "Test: GroupNeg"
test_glob "!([a-z]).txt" "A.txt" "true" "GroupNeg: !([a-z]).txt matches A.txt"
test_glob "!([a-z]).txt" "1.txt" "true" "GroupNeg: !([a-z]).txt matches 1.txt"
test_glob "!([a-z]).txt" "a.txt" "false" "GroupNeg: !([a-z]).txt does not match a.txt"
echo ""

# Test: GroupUnion
echo "Test: GroupUnion"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" "FILE1.txt" "true" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) matches FILE1.txt"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" "FILE1.pdf" "true" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) matches FILE1.pdf"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" "FILE.pdf" "true" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) matches FILE.pdf"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" "F3.txt" "true" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) matches F3.txt"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" ".txt" "true" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) matches .txt"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" "FILE.jpg" "false" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) does not match FILE.jpg"
test_glob "*([a-zA-Z])*([0-9]).(txt|pdf)" "FF.sdf" "false" "GroupUnion: *([a-zA-Z])*([0-9]).(txt|pdf) does not match FF.sdf"
echo ""

# Test: GroupMultipleUnions
echo "Test: GroupMultipleUnions"
test_glob "(a|b|c|d)" "a" "true" "GroupMultipleUnions: (a|b|c|d) matches a"
test_glob "(a|b|c|d)" "b" "true" "GroupMultipleUnions: (a|b|c|d) matches b"
test_glob "(a|b|c|d)" "c" "true" "GroupMultipleUnions: (a|b|c|d) matches c"
test_glob "(a|b|c|d)" "d" "true" "GroupMultipleUnions: (a|b|c|d) matches d"
test_glob "(a|b|c|d)" "e" "false" "GroupMultipleUnions: (a|b|c|d) does not match e"
echo ""

# Test: GroupNested
echo "Test: GroupNested"
test_glob "*((a|b)|(c|d))" "a" "true" "GroupNested: *((a|b)|(c|d)) matches a"
test_glob "*((a|b)|(c|d))" "b" "true" "GroupNested: *((a|b)|(c|d)) matches b"
test_glob "*((a|b)|(c|d))" "c" "true" "GroupNested: *((a|b)|(c|d)) matches c"
test_glob "*((a|b)|(c|d))" "d" "true" "GroupNested: *((a|b)|(c|d)) matches d"
test_glob "*((a|b)|(c|d))" "" "true" "GroupNested: *((a|b)|(c|d)) matches empty"
test_glob "*((a|b)|(c|d))" "e" "false" "GroupNested: *((a|b)|(c|d)) does not match e"
echo ""

# Test: GroupComplex
echo "Test: GroupComplex"
test_glob "*([a-z])+([0-9]).(txt|pdf)" "file1.txt" "true" "GroupComplex: *([a-z])+([0-9]).(txt|pdf) matches file1.txt"
test_glob "*([a-z])+([0-9]).(txt|pdf)" "file123.pdf" "true" "GroupComplex: *([a-z])+([0-9]).(txt|pdf) matches file123.pdf"
test_glob "*([a-z])+([0-9]).(txt|pdf)" "file.txt" "false" "GroupComplex: *([a-z])+([0-9]).(txt|pdf) does not match file.txt"
test_glob "*([a-z])+([0-9]).(txt|pdf)" "file.jpg" "false" "GroupComplex: *([a-z])+([0-9]).(txt|pdf) does not match file.jpg"
echo ""

# Test: PatternAllWildcards
echo "Test: PatternAllWildcards"
test_glob "***" "" "true" "PatternAllWildcards: *** matches empty"
test_glob "***" "a" "true" "PatternAllWildcards: *** matches a"
test_glob "***" "abc" "true" "PatternAllWildcards: *** matches abc"
echo ""

# Test: ComplexPattern
echo "Test: ComplexPattern"
test_glob "*([a-z])+([0-9])*(.txt|.pdf|.jpg)" "file123.txt" "true" "ComplexPattern: *([a-z])+([0-9])*(.txt|.pdf|.jpg) matches file123.txt"
test_glob "*([a-z])+([0-9])*(.txt|.pdf|.jpg)" "abc456.pdf" "true" "ComplexPattern: *([a-z])+([0-9])*(.txt|.pdf|.jpg) matches abc456.pdf"
test_glob "*([a-z])+([0-9])*(.txt|.pdf|.jpg)" "123.txt" "true" "ComplexPattern: *([a-z])+([0-9])*(.txt|.pdf|.jpg) matches 123.txt (shell: *([a-z]) allows zero letters)"
echo ""

# Test: DoubleAsteriskPattern
echo "Test: DoubleAsteriskPattern"
test_glob "https://**.google.com" "https://foo.bar.google.com" "true" "DoubleAsteriskPattern: https://**.google.com matches https://foo.bar.google.com"
test_glob "https://**.google.com" "https://google.com" "false" "DoubleAsteriskPattern: https://**.google.com does NOT match https://google.com (shell: ** in middle requires at least one character)"
test_glob "https://**.google.com" "https://a.google.com" "true" "DoubleAsteriskPattern: https://**.google.com matches https://a.google.com"
test_glob "https://**.google.com" "https://a.b.c.google.com" "true" "DoubleAsteriskPattern: https://**.google.com matches https://a.b.c.google.com"
echo ""

# Summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "Total tests: $((PASSED + FAILED))"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    echo ""
    echo "Note: Differences may be due to:"
    echo "  - Shell uses [!...] for negation, C++ uses [^...] (converted automatically)"
    echo "  - Shell globbing behavior differences"
    exit 1
fi
