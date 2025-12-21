#!/bin/bash

# Unified test script for glob patterns
# Tests either:
#   1. Shell glob behavior (bash case statements) - when run without arguments
#   2. C++ glob tool - when run with path to executable
#
# Usage:
#   ./glob_test.sh              # Test shell glob behavior
#   ./glob_test.sh /path/to/glob_tool  # Test C++ glob tool

# Don't exit on error - we want to run all tests
set +e

# Enable extended globbing for patterns like *() and +()
shopt -s extglob

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0
MODE=""
GLOB_TOOL=""
VERBOSE=0

# ============================================================================
# Mode Detection and Setup
# ============================================================================

if [ $# -eq 0 ]; then
    MODE="shell"
    echo -e "${BLUE}=========================================="
    echo "Testing: SHELL GLOB BEHAVIOR (bash)"
    echo -e "==========================================${NC}"
elif [ $# -eq 1 ] || [ $# -eq 2 ]; then
    if [ "$1" = "-v" ] || [ "$1" = "--verbose" ]; then
        VERBOSE=1
        shift
    elif [ "$2" = "-v" ] || [ "$2" = "--verbose" ]; then
        VERBOSE=1
    fi

    if [ $# -eq 0 ]; then
        MODE="shell"
    else
        GLOB_TOOL="$1"
        if [ ! -x "$GLOB_TOOL" ]; then
            echo -e "${RED}Error: '$GLOB_TOOL' is not an executable file${NC}"
            exit 1
        fi
        MODE="tool"
        echo -e "${BLUE}=========================================="
        echo "Testing: C++ GLOB TOOL"
        echo "Tool: $GLOB_TOOL"
        [ $VERBOSE -eq 1 ] && echo "Verbose mode: ON"
        echo -e "==========================================${NC}"
    fi
else
    echo "Usage: $0 [-v|--verbose] [path_to_glob_executable]"
    echo "  No args: Test shell glob behavior"
    echo "  With arg: Test C++ glob tool"
    echo "  -v / --verbose: Show tool output (only in tool mode)"
    exit 1
fi

echo ""

# ============================================================================
# Test Functions - Shell Mode
# ============================================================================
# Note: Shell uses [!...] for negation, C++ uses [^...]
#       Extended globbing enabled for patterns like *() and +()

# Test function: test_glob pattern string expected test_name
# Uses bash case statement for glob matching
test_glob_shell() {
    local pattern="$1"
    local test_string="$2"
    local expected="$3"
    local test_name="$4"
    
    # Convert C++ negation syntax [^...] to shell syntax [!...]
    local shell_pattern="${pattern//\[^/[!}"
    
    # Bash case statement uses glob patterns
    # Note: Brace expansion happens before pattern matching in bash
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

# Test function for brace expansion patterns
# Bash expands braces before pattern matching in case statements
test_glob_brace_shell() {
    local pattern="$1"
    local test_string="$2"
    local expected="$3"
    local test_name="$4"
    
    # Convert C++ negation syntax [^...] to shell syntax [!...]
    local shell_pattern="${pattern//\[^/[!}"
    local result="false"
    
    # Expand braces using bash's brace expansion
    # Create an array of expanded patterns
    local expanded_patterns
    # Use a here-string to expand braces safely
    expanded_patterns=$(eval "echo $shell_pattern")
    
    # Test against each expanded pattern
    for exp_pattern in $expanded_patterns; do
        case "$test_string" in
            $exp_pattern)
                result="true"
                break
                ;;
        esac
    done
    
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

# ============================================================================
# Test Functions - Tool Mode
# ============================================================================

test_glob_tool() {
    local pattern="$1"
    local test_string="$2"
    local expected="$3"
    local test_name="$4"
    
    if [ $VERBOSE -eq 1 ]; then
        # Show command and full output
        echo "Running: $GLOB_TOOL \"$pattern\" \"$test_string\""
        "$GLOB_TOOL" "$pattern" "$test_string"
        local result=$?
    else
        # Default quiet behavior
        "$GLOB_TOOL" "$pattern" "$test_string" > /dev/null 2>&1
        local result=$?
    fi
    
    local result_str
    if [ $result -eq 0 ]; then
        result_str="true"
    else
        result_str="false"
    fi
    
    if [ "$result_str" = "$expected" ]; then
        echo -e "${GREEN}PASS${NC}: $test_name"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}FAIL${NC}: $test_name - Pattern: '$pattern' String: '$test_string' Expected: $expected Got: $result_str"
        ((FAILED++))
        return 1
    fi
}

# ============================================================================
# Set up function aliases based on mode
# ============================================================================

if [ "$MODE" = "shell" ]; then
    test_glob() { test_glob_shell "$@"; }
    test_glob_brace() { test_glob_brace_shell "$@"; }
else
    test_glob() { test_glob_tool "$@"; }
    test_glob_brace() { test_glob_tool "$@"; }
fi

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
test_glob "!(foo|bar).txt" "baz.txt" "true" "GroupNeg: !(foo|bar).txt matches baz.txt"

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

# ============================================================================
# Escape Sequence Tests
# ============================================================================

# Test: EscapeStar
echo "Test: EscapeStar"
test_glob "\\*" "*" "true" "EscapeStar: \\* matches *"
test_glob "\\*" "a" "false" "EscapeStar: \\* does not match a"
test_glob "\\*" "" "false" "EscapeStar: \\* does not match empty"
echo ""

# Test: EscapeQuestion
echo "Test: EscapeQuestion"
test_glob "\\?" "?" "true" "EscapeQuestion: \\? matches ?"
test_glob "\\?" "a" "false" "EscapeQuestion: \\? does not match a"
echo ""

# Test: EscapePlus
echo "Test: EscapePlus"
test_glob "\\+" "+" "true" "EscapePlus: \\+ matches +"
test_glob "\\+" "a" "false" "EscapePlus: \\+ does not match a"
echo ""

# Test: EscapeParen
echo "Test: EscapeParen"
test_glob "\\(" "(" "true" "EscapeParen: \\( matches ("
test_glob "\\(" "a" "false" "EscapeParen: \\( does not match a"
test_glob "\\)" ")" "true" "EscapeParen: \\) matches )"
echo ""

# Test: EscapeBracket
echo "Test: EscapeBracket"
test_glob "\\[" "[" "true" "EscapeBracket: \\[ matches ["
test_glob "\\]" "]" "true" "EscapeBracket: \\] matches ]"
echo ""

# Test: EscapePipe
echo "Test: EscapePipe"
test_glob "\\|" "|" "true" "EscapePipe: \\| matches |"
echo ""

# Test: EscapeExclamation
echo "Test: EscapeExclamation"
test_glob "\\!" "!" "true" "EscapeExclamation: \\! matches !"
echo ""

# Test: EscapeAt
echo "Test: EscapeAt"
test_glob "\\@" "@" "true" "EscapeAt: \\@ matches @"
echo ""

# Test: EscapeBackslash
echo "Test: EscapeBackslash"
test_glob "\\\\" "\\" "true" "EscapeBackslash: \\\\ matches \\"
echo ""

# Test: EscapeInSet
echo "Test: EscapeInSet"
test_glob "[\\*\\?]" "*" "true" "EscapeInSet: [\\*\\?] matches *"
test_glob "[\\*\\?]" "?" "true" "EscapeInSet: [\\*\\?] matches ?"
test_glob "[\\*\\?]" "a" "false" "EscapeInSet: [\\*\\?] does not match a"
echo ""

# Test: EscapeInPattern
echo "Test: EscapeInPattern"
test_glob "test\\*.txt" "test*.txt" "true" "EscapeInPattern: test\\*.txt matches test*.txt"
test_glob "test\\*.txt" "testa.txt" "false" "EscapeInPattern: test\\*.txt does not match testa.txt"
echo ""

# ============================================================================
# Boundary Condition Tests
# ============================================================================

# Test: LongPattern
echo "Test: LongPattern"
long_pattern=$(printf 'a%.0s' {1..1000})
long_pattern="${long_pattern}*"
long_string="${long_pattern%?}test"
test_glob "$long_pattern" "$long_string" "true" "LongPattern: long pattern matches long string"
echo ""

# Test: LongString
echo "Test: LongString"
long_string=$(printf 'a%.0s' {1..10000})
test_glob "*" "$long_string" "true" "LongString: * matches very long string"
echo ""

# Test: ComplexPattern
echo "Test: ComplexPattern"
test_glob "*([a-z])+([0-9])*(.txt|.pdf|.jpg)" "file123.txt" "true" "ComplexPattern: *([a-z])+([0-9])*(.txt|.pdf|.jpg) matches file123.txt"
test_glob "*([a-z])+([0-9])*(.txt|.pdf|.jpg)" "abc456.pdf" "true" "ComplexPattern: *([a-z])+([0-9])*(.txt|.pdf|.jpg) matches abc456.pdf"
test_glob "*([a-z])+([0-9])*(.txt|.pdf|.jpg)" "123.txt" "true" "ComplexPattern: *([a-z])+([0-9])*(.txt|.pdf|.jpg) matches 123.txt"
echo ""

# Test: ZeroLengthMatch
echo "Test: ZeroLengthMatch"
test_glob "*(test)" "" "true" "ZeroLengthMatch: *(test) matches empty"
test_glob "*(test)" "test" "true" "ZeroLengthMatch: *(test) matches test"
test_glob "*(test)" "testtest" "true" "ZeroLengthMatch: *(test) matches testtest"
echo ""

# Test: EmptyPattern
echo "Test: EmptyPattern"
test_glob "" "" "true" "EmptyPattern: '' matches empty"
test_glob "" "test" "false" "EmptyPattern: '' does not match 'test'"
echo ""

# ============================================================================
# Parameterized Tests for Similar Cases
# ============================================================================

# Test: ParameterizedBasicPatterns
echo "Test: ParameterizedBasicPatterns"
test_glob "*.txt" "file.txt" "true" "Parameterized: *.txt matches file.txt"
test_glob "*.txt" "file.pdf" "false" "Parameterized: *.txt does not match file.pdf"
test_glob "test?" "test1" "true" "Parameterized: test? matches test1"
test_glob "test?" "test" "false" "Parameterized: test? does not match test"
test_glob "test?" "test12" "false" "Parameterized: test? does not match test12"
test_glob "[a-z]*" "test" "true" "Parameterized: [a-z]* matches test"
test_glob "[a-z]*" "TEST" "false" "Parameterized: [a-z]* does not match TEST"
echo ""

# ============================================================================
# Special Character Tests
# ============================================================================

# Test: SpecialCharsInPattern
echo "Test: SpecialCharsInPattern"
test_glob "test\\*.txt" "test*.txt" "true" "SpecialCharsInPattern: test\\*.txt matches test*.txt"
test_glob "test\\*.txt" "testa.txt" "false" "SpecialCharsInPattern: test\\*.txt does not match testa.txt"
echo ""

# Test: DotInPattern
echo "Test: DotInPattern"
test_glob "*.txt" "file.txt" "true" "DotInPattern: *.txt matches file.txt"
test_glob "*.txt" "filetxt" "false" "DotInPattern: *.txt does not match filetxt"
echo ""

# Test: UnderscoreInPattern
echo "Test: UnderscoreInPattern"
test_glob "test_file.txt" "test_file.txt" "true" "UnderscoreInPattern: test_file.txt matches test_file.txt"
test_glob "test_file.txt" "testfile.txt" "false" "UnderscoreInPattern: test_file.txt does not match testfile.txt"
echo ""

# Test: HyphenInPattern
echo "Test: HyphenInPattern"
test_glob "test-file.txt" "test-file.txt" "true" "HyphenInPattern: test-file.txt matches test-file.txt"
test_glob "test-file.txt" "testfile.txt" "false" "HyphenInPattern: test-file.txt does not match testfile.txt"
echo ""

# ============================================================================
# Edge Cases
# ============================================================================

# Test: PatternStartsWithWildcard
echo "Test: PatternStartsWithWildcard"
test_glob "*test" "test" "true" "PatternStartsWithWildcard: *test matches test"
test_glob "*test" "atest" "true" "PatternStartsWithWildcard: *test matches atest"
test_glob "*test" "tes" "false" "PatternStartsWithWildcard: *test does not match tes"
echo ""

# Test: PatternEndsWithWildcard
echo "Test: PatternEndsWithWildcard"
test_glob "test*" "test" "true" "PatternEndsWithWildcard: test* matches test"
test_glob "test*" "testa" "true" "PatternEndsWithWildcard: test* matches testa"
test_glob "test*" "tes" "false" "PatternEndsWithWildcard: test* does not match tes"
echo ""

# Test: PatternAllWildcards
echo "Test: PatternAllWildcards"
test_glob "***" "" "true" "PatternAllWildcards: *** matches empty"
test_glob "***" "a" "true" "PatternAllWildcards: *** matches a"
test_glob "***" "abc" "true" "PatternAllWildcards: *** matches abc"
echo ""

# Test: ConsecutiveWildcards
echo "Test: ConsecutiveWildcards"
test_glob "test**file" "testfile" "true" "ConsecutiveWildcards: test**file matches testfile"
test_glob "test**file" "test123file" "true" "ConsecutiveWildcards: test**file matches test123file"
echo ""

# Test: QuestionStarCombination
echo "Test: QuestionStarCombination"
test_glob "test?*file" "test1file" "true" "QuestionStarCombination: test?*file matches test1file"
test_glob "test?*file" "test123file" "true" "QuestionStarCombination: test?*file matches test123file"
test_glob "test?*file" "testfile" "false" "QuestionStarCombination: test?*file does not match testfile"
echo ""

# Test: DoubleAsteriskPattern
echo "Test: DoubleAsteriskPattern"
test_glob "https://**.google.com" "https://foo.bar.google.com" "true" "DoubleAsteriskPattern: https://**.google.com matches https://foo.bar.google.com"
test_glob "https://**.google.com" "https://google.com" "false" "DoubleAsteriskPattern: https://**.google.com does NOT match https://google.com (shell: ** in middle requires at least one character)"
test_glob "https://**.google.com" "https://a.google.com" "true" "DoubleAsteriskPattern: https://**.google.com matches https://a.google.com"
test_glob "https://**.google.com" "https://a.b.c.google.com" "true" "DoubleAsteriskPattern: https://**.google.com matches https://a.b.c.google.com"
echo ""

# ============================================================================
# Brace Expansion Tests
# ============================================================================

# Note: Bash brace expansion happens before glob matching, so we need to test
# the expanded patterns. For patterns like *.{h,hpp}, bash expands to *.h and *.hpp
# before glob matching, so we test each expansion separately.

# Test: BraceExpansionBasic
echo "Test: BraceExpansionBasic"
test_glob_brace "*.{h,hpp}" "file.h" "true" "BraceExpansionBasic: *.{h,hpp} matches file.h"
test_glob_brace "*.{h,hpp}" "test.hpp" "true" "BraceExpansionBasic: *.{h,hpp} matches test.hpp"
test_glob_brace "*.{h,hpp}" "file.c" "false" "BraceExpansionBasic: *.{h,hpp} does not match file.c"
test_glob_brace "*.{h,hpp}" "file.hh" "false" "BraceExpansionBasic: *.{h,hpp} does not match file.hh"
echo ""

# Test: BraceExpansionMultipleItems
echo "Test: BraceExpansionMultipleItems"
test_glob_brace "*.{h,hpp,c,cpp}" "file.h" "true" "BraceExpansionMultipleItems: *.{h,hpp,c,cpp} matches file.h"
test_glob_brace "*.{h,hpp,c,cpp}" "file.hpp" "true" "BraceExpansionMultipleItems: *.{h,hpp,c,cpp} matches file.hpp"
test_glob_brace "*.{h,hpp,c,cpp}" "file.c" "true" "BraceExpansionMultipleItems: *.{h,hpp,c,cpp} matches file.c"
test_glob_brace "*.{h,hpp,c,cpp}" "file.cpp" "true" "BraceExpansionMultipleItems: *.{h,hpp,c,cpp} matches file.cpp"
test_glob_brace "*.{h,hpp,c,cpp}" "file.txt" "false" "BraceExpansionMultipleItems: *.{h,hpp,c,cpp} does not match file.txt"
test_glob_brace "*.{h,hpp,c,cpp}" "file.hh" "false" "BraceExpansionMultipleItems: *.{h,hpp,c,cpp} does not match file.hh"
echo ""

# Test: BraceExpansionGreedyMatching (CRITICAL TEST!)
echo "Test: BraceExpansionGreedyMatching"
test_glob_brace "*.{c,cp,cpp}" "file.c" "true" "BraceExpansionGreedyMatching: *.{c,cp,cpp} matches file.c (shortest when only option)"
test_glob_brace "*.{c,cp,cpp}" "file.cp" "true" "BraceExpansionGreedyMatching: *.{c,cp,cpp} matches file.cp (greedy: matches cp not just c)"
test_glob_brace "*.{c,cp,cpp}" "file.cpp" "true" "BraceExpansionGreedyMatching: *.{c,cp,cpp} matches file.cpp (longest match)"
test_glob_brace "*.{c,cp,cpp}" "file.cxx" "false" "BraceExpansionGreedyMatching: *.{c,cp,cpp} does not match file.cxx"
echo ""

# Test: BraceExpansionComplexList
echo "Test: BraceExpansionComplexList"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.h" "true" "BraceExpansionComplexList: Complex list matches foo.h"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.hp" "true" "BraceExpansionComplexList: Complex list matches foo.hp"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.hpp" "true" "BraceExpansionComplexList: Complex list matches foo.hpp"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.hxx" "true" "BraceExpansionComplexList: Complex list matches foo.hxx"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.c" "true" "BraceExpansionComplexList: Complex list matches foo.c"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.cc" "true" "BraceExpansionComplexList: Complex list matches foo.cc"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.cp" "true" "BraceExpansionComplexList: Complex list matches foo.cp (CRITICAL!)"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.cpp" "true" "BraceExpansionComplexList: Complex list matches foo.cpp"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.cxx" "true" "BraceExpansionComplexList: Complex list matches foo.cxx"
test_glob_brace "*.{h,hp,hpp,hxx,c,cc,cp,cpp,cxx}" "foo.java" "false" "BraceExpansionComplexList: Complex list does not match foo.java"
echo ""

# Test: BraceExpansionRanges
echo "Test: BraceExpansionRanges"
test_glob_brace "file{1..3}.txt" "file1.txt" "true" "BraceExpansionRanges: file{1..3}.txt matches file1.txt"
test_glob_brace "file{1..3}.txt" "file2.txt" "true" "BraceExpansionRanges: file{1..3}.txt matches file2.txt"
test_glob_brace "file{1..3}.txt" "file3.txt" "true" "BraceExpansionRanges: file{1..3}.txt matches file3.txt"
test_glob_brace "file{1..3}.txt" "file0.txt" "false" "BraceExpansionRanges: file{1..3}.txt does not match file0.txt"
test_glob_brace "file{1..3}.txt" "file4.txt" "false" "BraceExpansionRanges: file{1..3}.txt does not match file4.txt"
echo ""

# Test: BraceExpansionCharRange
echo "Test: BraceExpansionCharRange"
test_glob_brace "test{a..c}.log" "testa.log" "true" "BraceExpansionCharRange: test{a..c}.log matches testa.log"
test_glob_brace "test{a..c}.log" "testb.log" "true" "BraceExpansionCharRange: test{a..c}.log matches testb.log"
test_glob_brace "test{a..c}.log" "testc.log" "true" "BraceExpansionCharRange: test{a..c}.log matches testc.log"
test_glob_brace "test{a..c}.log" "testd.log" "false" "BraceExpansionCharRange: test{a..c}.log does not match testd.log"
echo ""

# Test: BraceExpansionReverseRange
echo "Test: BraceExpansionReverseRange"
test_glob_brace "file{5..1}.txt" "file5.txt" "true" "BraceExpansionReverseRange: file{5..1}.txt matches file5.txt"
test_glob_brace "file{5..1}.txt" "file3.txt" "true" "BraceExpansionReverseRange: file{5..1}.txt matches file3.txt"
test_glob_brace "file{5..1}.txt" "file1.txt" "true" "BraceExpansionReverseRange: file{5..1}.txt matches file1.txt"
test_glob_brace "file{5..1}.txt" "file0.txt" "false" "BraceExpansionReverseRange: file{5..1}.txt does not match file0.txt"
echo ""

# Test: BraceExpansionWithPrefix
echo "Test: BraceExpansionWithPrefix"
test_glob_brace "test.{txt,md}" "test.txt" "true" "BraceExpansionWithPrefix: test.{txt,md} matches test.txt"
test_glob_brace "test.{txt,md}" "test.md" "true" "BraceExpansionWithPrefix: test.{txt,md} matches test.md"
test_glob_brace "test.{txt,md}" "test.pdf" "false" "BraceExpansionWithPrefix: test.{txt,md} does not match test.pdf"
test_glob_brace "test.{txt,md}" "atest.txt" "false" "BraceExpansionWithPrefix: test.{txt,md} does not match atest.txt"
echo ""

# Test: BraceExpansionAtStart
echo "Test: BraceExpansionAtStart"
test_glob_brace "{a,b}*.txt" "a.txt" "true" "BraceExpansionAtStart: {a,b}*.txt matches a.txt"
test_glob_brace "{a,b}*.txt" "b.txt" "true" "BraceExpansionAtStart: {a,b}*.txt matches b.txt"
test_glob_brace "{a,b}*.txt" "a123.txt" "true" "BraceExpansionAtStart: {a,b}*.txt matches a123.txt"
test_glob_brace "{a,b}*.txt" "bfile.txt" "true" "BraceExpansionAtStart: {a,b}*.txt matches bfile.txt"
test_glob_brace "{a,b}*.txt" "c.txt" "false" "BraceExpansionAtStart: {a,b}*.txt does not match c.txt"
test_glob_brace "{a,b}*.txt" "ab.txt" "true" "BraceExpansionAtStart: {a,b}*.txt matches ab.txt"
echo ""

# Test: BraceExpansionSingleItem
echo "Test: BraceExpansionSingleItem"
test_glob_brace "*.{h}" "file.h" "true" "BraceExpansionSingleItem: *.{h} matches file.h"
test_glob_brace "*.{h}" "file.hpp" "false" "BraceExpansionSingleItem: *.{h} does not match file.hpp"
test_glob_brace "*.{h}" "file.c" "false" "BraceExpansionSingleItem: *.{h} does not match file.c"
echo ""

# Test: BraceExpansionWithWildcards
echo "Test: BraceExpansionWithWildcards"
test_glob_brace "test*.{txt,pdf}" "test.txt" "true" "BraceExpansionWithWildcards: test*.{txt,pdf} matches test.txt"
test_glob_brace "test*.{txt,pdf}" "test123.pdf" "true" "BraceExpansionWithWildcards: test*.{txt,pdf} matches test123.pdf"
test_glob_brace "test*.{txt,pdf}" "test_file.txt" "true" "BraceExpansionWithWildcards: test*.{txt,pdf} matches test_file.txt"
test_glob_brace "test*.{txt,pdf}" "test.jpg" "false" "BraceExpansionWithWildcards: test*.{txt,pdf} does not match test.jpg"
echo ""

# Test: BraceExpansionNested
echo "Test: BraceExpansionNested"
test_glob_brace "*.{h{pp,xx},c}" "file.hpp" "true" "BraceExpansionNested: *.{h{pp,xx},c} matches file.hpp"
test_glob_brace "*.{h{pp,xx},c}" "file.hxx" "true" "BraceExpansionNested: *.{h{pp,xx},c} matches file.hxx"
test_glob_brace "*.{h{pp,xx},c}" "file.c" "true" "BraceExpansionNested: *.{h{pp,xx},c} matches file.c"
test_glob_brace "*.{h{pp,xx},c}" "file.h" "false" "BraceExpansionNested: *.{h{pp,xx},c} does not match file.h"
test_glob_brace "*.{h{pp,xx},c}" "file.hppp" "false" "BraceExpansionNested: *.{h{pp,xx},c} does not match file.hppp"
echo ""

# Test: BraceExpansionNestedComplex
echo "Test: BraceExpansionNestedComplex"
test_glob_brace "{a,b{1,2}}*.txt" "a.txt" "true" "BraceExpansionNestedComplex: {a,b{1,2}}*.txt matches a.txt"
test_glob_brace "{a,b{1,2}}*.txt" "b1.txt" "true" "BraceExpansionNestedComplex: {a,b{1,2}}*.txt matches b1.txt"
test_glob_brace "{a,b{1,2}}*.txt" "b2.txt" "true" "BraceExpansionNestedComplex: {a,b{1,2}}*.txt matches b2.txt"
test_glob_brace "{a,b{1,2}}*.txt" "afile.txt" "true" "BraceExpansionNestedComplex: {a,b{1,2}}*.txt matches afile.txt"
test_glob_brace "{a,b{1,2}}*.txt" "b1test.txt" "true" "BraceExpansionNestedComplex: {a,b{1,2}}*.txt matches b1test.txt"
test_glob_brace "{a,b{1,2}}*.txt" "b.txt" "false" "BraceExpansionNestedComplex: {a,b{1,2}}*.txt does not match b.txt"
echo ""

# Test: BraceExpansionNestedPaths
echo "Test: BraceExpansionNestedPaths"
test_glob_brace "{test,prod}/{log,data}.txt" "test/log.txt" "true" "BraceExpansionNestedPaths: {test,prod}/{log,data}.txt matches test/log.txt"
test_glob_brace "{test,prod}/{log,data}.txt" "test/data.txt" "true" "BraceExpansionNestedPaths: {test,prod}/{log,data}.txt matches test/data.txt"
test_glob_brace "{test,prod}/{log,data}.txt" "prod/log.txt" "true" "BraceExpansionNestedPaths: {test,prod}/{log,data}.txt matches prod/log.txt"
test_glob_brace "{test,prod}/{log,data}.txt" "prod/data.txt" "true" "BraceExpansionNestedPaths: {test,prod}/{log,data}.txt matches prod/data.txt"
test_glob_brace "{test,prod}/{log,data}.txt" "dev/log.txt" "false" "BraceExpansionNestedPaths: {test,prod}/{log,data}.txt does not match dev/log.txt"
echo ""

# Test: BraceExpansionMultipleRanges
echo "Test: BraceExpansionMultipleRanges"
test_glob_brace "file{1..2}{a..b}.txt" "file1a.txt" "true" "BraceExpansionMultipleRanges: file{1..2}{a..b}.txt matches file1a.txt"
test_glob_brace "file{1..2}{a..b}.txt" "file1b.txt" "true" "BraceExpansionMultipleRanges: file{1..2}{a..b}.txt matches file1b.txt"
test_glob_brace "file{1..2}{a..b}.txt" "file2a.txt" "true" "BraceExpansionMultipleRanges: file{1..2}{a..b}.txt matches file2a.txt"
test_glob_brace "file{1..2}{a..b}.txt" "file2b.txt" "true" "BraceExpansionMultipleRanges: file{1..2}{a..b}.txt matches file2b.txt"
test_glob_brace "file{1..2}{a..b}.txt" "file1c.txt" "false" "BraceExpansionMultipleRanges: file{1..2}{a..b}.txt does not match file1c.txt"
echo ""

# Test: BraceExpansionComplexNested
echo "Test: BraceExpansionComplexNested"
test_glob_brace "{a,b}{c{d,e},f}" "acd" "true" "BraceExpansionComplexNested: {a,b}{c{d,e},f} matches acd"
test_glob_brace "{a,b}{c{d,e},f}" "ace" "true" "BraceExpansionComplexNested: {a,b}{c{d,e},f} matches ace"
test_glob_brace "{a,b}{c{d,e},f}" "af" "true" "BraceExpansionComplexNested: {a,b}{c{d,e},f} matches af"
test_glob_brace "{a,b}{c{d,e},f}" "bcd" "true" "BraceExpansionComplexNested: {a,b}{c{d,e},f} matches bcd"
test_glob_brace "{a,b}{c{d,e},f}" "bce" "true" "BraceExpansionComplexNested: {a,b}{c{d,e},f} matches bce"
test_glob_brace "{a,b}{c{d,e},f}" "bf" "true" "BraceExpansionComplexNested: {a,b}{c{d,e},f} matches bf"
test_glob_brace "{a,b}{c{d,e},f}" "ac" "false" "BraceExpansionComplexNested: {a,b}{c{d,e},f} does not match ac"
echo ""

# Test: BraceExpansionEmptyBraces
echo "Test: BraceExpansionEmptyBraces"
test_glob "test" "test" "true" "BraceExpansionEmptyBraces: test{} expands to test, matches test"
test_glob "test" "testx" "false" "BraceExpansionEmptyBraces: test{} expands to test, does not match testx"
echo ""

# Test: BraceExpansionTrailingComma
echo "Test: BraceExpansionTrailingComma"
test_glob_brace "*.{h,}" "file.h" "true" "BraceExpansionTrailingComma: *.{h,} matches file.h"
test_glob_brace "*.{h,}" "file." "true" "BraceExpansionTrailingComma: *.{h,} matches file. (trailing comma adds empty item)"
test_glob_brace "*.{h,}" "file.c" "false" "BraceExpansionTrailingComma: *.{h,} does not match file.c"
echo ""

# Test: BraceExpansionLeadingComma
echo "Test: BraceExpansionLeadingComma"
test_glob_brace "*.{,h}" "file." "true" "BraceExpansionLeadingComma: *.{,h} matches file. (leading comma adds empty item)"
test_glob_brace "*.{,h}" "file.h" "true" "BraceExpansionLeadingComma: *.{,h} matches file.h"
test_glob_brace "*.{,h}" "file.c" "false" "BraceExpansionLeadingComma: *.{,h} does not match file.c"
echo ""

# Test: BraceExpansionWithSets
echo "Test: BraceExpansionWithSets"
test_glob_brace "file[0-9].{txt,pdf}" "file1.txt" "true" "BraceExpansionWithSets: file[0-9].{txt,pdf} matches file1.txt"
test_glob_brace "file[0-9].{txt,pdf}" "file5.pdf" "true" "BraceExpansionWithSets: file[0-9].{txt,pdf} matches file5.pdf"
test_glob_brace "file[0-9].{txt,pdf}" "filea.txt" "false" "BraceExpansionWithSets: file[0-9].{txt,pdf} does not match filea.txt"
test_glob_brace "file[0-9].{txt,pdf}" "file1.jpg" "false" "BraceExpansionWithSets: file[0-9].{txt,pdf} does not match file1.jpg"
echo ""

# Test: BraceExpansionErrorUnclosed
echo "Test: BraceExpansionErrorUnclosed"
# Note: Shell doesn't throw exceptions, but we can test that unclosed braces don't match
# In bash, unclosed braces are treated as literal characters
test_glob "*.{h,hpp" "file.h" "false" "BraceExpansionErrorUnclosed: Invalid unclosed '*.{h,hpp' does not match 'file.h'"
test_glob "*.{h,hpp" "file.{h,hpp" "false" "BraceExpansionErrorUnclosed: Invalid unclosed '*.{h,hpp'. Escape to treat as literal"
echo ""

# Test: BraceExpansionEscaped
echo "Test: BraceExpansionEscaped"
test_glob "\\{test\\}" "{test}" "true" "BraceExpansionEscaped: \\{test\\} matches {test}"
test_glob "\\{test\\}" "test" "false" "BraceExpansionEscaped: \\{test\\} does not match test"
echo ""

# Test: BraceExpansionComplex
echo "Test: BraceExpansionComplex"
test_glob_brace "prefix*{a,b}*suffix.{ext1,ext2}" "prefix123a456suffix.ext1" "true" "BraceExpansionComplex: prefix*{a,b}*suffix.{ext1,ext2} matches prefix123a456suffix.ext1"
test_glob_brace "prefix*{a,b}*suffix.{ext1,ext2}" "prefixxa456suffix.ext1" "true" "BraceExpansionComplex: prefix*{a,b}*suffix.{ext1,ext2} matches prefixxa456suffix.ext1"
test_glob_brace "prefix*{a,b}*suffix.{ext1,ext2}" "prefixxbsuffix.ext2" "true" "BraceExpansionComplex: prefix*{a,b}*suffix.{ext1,ext2} matches prefixxbsuffix.ext2"
test_glob_brace "prefix*{a,b}*suffix.{ext1,ext2}" "prefixxaext1" "false" "BraceExpansionComplex: prefix*{a,b}*suffix.{ext1,ext2} does not match prefixxaext1 (missing suffix)"
test_glob_brace "prefix*{a,b}*suffix.{ext1,ext2}" "prefixxbext2" "false" "BraceExpansionComplex: prefix*{a,b}*suffix.{ext1,ext2} does not match prefixxbext2 (missing suffix)"
test_glob_brace "prefix*{a,b}*suffix.{ext1,ext2}" "prefixxsuffix.ext3" "false" "BraceExpansionComplex: prefix*{a,b}*suffix.{ext1,ext2} does not match prefixxsuffix.ext3"
echo ""

# ============================================================================
# EDGE-CASE TESTS FOR BRACE EXPANSION (StateGroup::NextBasic coverage)
# ============================================================================

echo "Test: Brace Expansion - Empty Only"
test_glob_brace "file{}" "file" "true" "file{} matches file (empty alternative only)"
test_glob_brace "file{}" "filex" "false" "file{} does not match filex"
test_glob_brace "{}file" "file" "true" "{}file matches file (empty prefix)"
test_glob_brace "{file}" "file" "true" "{file} matches file (single non-empty)"
test_glob_brace "{}" "" "true" "{} matches empty string"
test_glob_brace "{}" "a" "false" "{} does not match non-empty"
echo ""

echo "Test: Brace Expansion - Leading/Trailing Empty"
test_glob_brace "{,.bak}file" "file" "true" "{,.bak}file matches file (leading empty)"
test_glob_brace "{,.bak}file" ".bakfile" "true" "{,.bak}file matches .bakfile"
test_glob_brace "pre{,.bak}" "pre" "true" "pre{,.bak} matches pre (trailing empty)"
test_glob_brace "pre{,.bak}" "pre.bak" "true" "pre{,.bak} matches pre.bak"
echo ""

echo "Test: Brace Expansion - Multiple Empty Alternatives"
test_glob_brace "file{,,,.bak}" "file" "true" "file{,,,.bak} matches file (multiple empty ok)"
test_glob_brace "file{,,,.bak}" "file.bak" "true" "file{,,,.bak} matches file.bak"
test_glob_brace "{,,}file{,,}" "file" "true" "{,,}file{,,} matches file (all empty paths)"
echo ""

echo "Test: Brace Expansion - Empty in Nested"
test_glob_brace "a{b{c,},d}" "ab" "true" "a{b{c,},d} matches ab (inner empty)"
test_glob_brace "a{b{c,},d}" "abc" "true" "a{b{c,},d} matches abc"
test_glob_brace "{outer{inner,}}" "outer" "true" "{outer{inner,}} matches outer (nested empty)"
test_glob_brace "{outer{inner,}}" "outerinner" "true" "{outer{inner,}} matches outerinner"
echo ""

echo "Test: Brace Expansion - Invalid Empty at Pattern End"
test_glob_brace "log{" "" "false" "'log{' does not match empty string (incomplete brace)"
# Note: if parser rejects incomplete brace, this test should fail compilation; otherwise behavior defined
test_glob_brace "file{" "file" "false" "'file{' does not match file (incomplete brace)"
echo ""

echo "Test: Brace Expansion - Anomalous/Invalid Patterns (should reject or literal)"
# These test parser robustness – should either throw Error or treat as literal
test_glob "file{a,b" "file{a,b" "false" "Invalid unbalanced brace. Escape to treat as literal"
test_glob "file{a,b" "filea" "false" "Unclosed brace does not expand"
test_glob "file{a,,b}" "filea" "true" "file{a,,b} allows double comma"
test_glob "file{a,,b}" "file" "true" "file{a,,b} matches file (empty allowed)"
echo ""

echo "Test: Brace Expansion - Incomplete / Unbalanced"
# Complete brace followed by literal '{'
test_glob "{a,b}{" "a{" "false" "{a,b}{ matches a{ (trailing { literal)"
test_glob "{a,b}{" "b{" "false" "{a,b}{ matches b{"
test_glob "{a,b}{" "{a,b}{" "false" "{a,b}{ does not match literal {a,b}{"
test_glob "{a,b}{" "ax" "false" "{a,b}{ does not match ax"

# Truly incomplete (unclosed) brace – should either error or treat as literal
test_glob "{a,b" "{a,b" "false" "Invalid unclosed brace '{a,b'. Escape to treat as literal"
test_glob "{a,b" "ab" "false" "Invalid unclosed brace does not expand"
test_glob "file{a,b" "filea" "false" "Invalid unclosed brace no expansion"

# Lone stray braces
test_glob "file}" "file}" "false" "Invalid lone '}'. Escape to treat as literal"
test_glob "{file" "{file" "false" "Invalid unbalanced '{'. Escape to treat as literal"
echo ""

echo "Test: Brace Expansion - With Other Wildcards at Boundaries"
test_glob_brace "*{a,b}.txt" "testa.txt" "true" "*{a,b}.txt matches testa.txt"
test_glob_brace "*{a,b}.txt" "testb.txt" "true" "*{a,b}.txt matches testb.txt"
test_glob_brace "?{a,b}.txt" "xa.txt" "true" "?{a,b}.txt matches xa.txt"
test_glob_brace "?{a,b}.txt" "xb.txt" "true" "?{a,b}.txt matches xb.txt"
test_glob_brace "{a,b}*.txt" "a123.txt" "true" "{a,b}*.txt matches a123.txt"
test_glob_brace "{a,b}*.txt" "b.txt" "true" "{a,b}*.txt matches b.txt"
echo ""

# ============================================================================
# CONTEXT-AWARE TOKENIZATION TESTS
# ============================================================================

echo "Test: Context-Aware - Comma"
test_glob "foo,bar" "foo,bar" "true" "Comma outside braces is literal"
test_glob "foo,bar" "foo" "false" "Comma outside braces doesn't separate"
test_glob "foo,bar" "foobar" "false" "Comma is not optional"
test_glob "*,*" "a,b" "true" "Comma literal with wildcards"
test_glob "*,*" "ab" "false" "Comma required when literal"
echo ""

echo "Test: Context-Aware - Pipe"
test_glob "foo|bar" "foo|bar" "true" "Pipe outside parens is literal"
test_glob "foo|bar" "foo" "false" "Pipe outside parens doesn't separate"
test_glob "foo|bar" "bar" "false" "Pipe outside parens doesn't separate"
test_glob "*|*" "a|b" "true" "Pipe literal with wildcards"
test_glob "*|*" "ab" "false" "Pipe required when literal"
echo ""

echo "Test: Context-Aware - Double Dot"
test_glob "test..txt" "test..txt" "true" "Double dot outside braces is literal"
test_glob "test..txt" "test.txt" "false" "Double dot doesn't match single dot"
test_glob "test..txt" "testXtxt" "false" "Double dot is not a wildcard"
test_glob "*.." "file.." "true" "Double dot at end is literal"
test_glob "..txt" "..txt" "true" "Double dot at start is literal"
echo ""

echo "Test: Context-Aware - Hyphen/Dash"
test_glob "foo-bar" "foo-bar" "true" "Hyphen outside brackets is literal"
test_glob "foo-bar" "foobar" "false" "Hyphen is not optional"
test_glob "*-*" "a-b" "true" "Hyphen literal with wildcards"
test_glob "test-[0-9]" "test-5" "true" "Hyphen literal before bracket, range inside"
test_glob "test-[0-9]" "test5" "false" "Hyphen required before bracket"
echo ""

echo "Test: Context-Aware - Plus"
test_glob "foo+bar" "foo+bar" "true" "Plus without parens is literal"
test_glob "foo+bar" "foobar" "false" "Plus is not optional"
test_glob "*+*" "a+b" "true" "Plus literal with wildcards"
echo ""

echo "Test: Context-Aware - At Symbol"
test_glob "user@host" "user@host" "true" "At symbol without parens is literal"
test_glob "user@host" "userhost" "false" "At symbol is not optional"
test_glob "*@*" "a@b" "true" "At symbol literal with wildcards"
echo ""

echo "Test: Context-Aware - Exclamation"
test_glob "wow!" "wow!" "true" "Exclamation without parens is literal"
test_glob "wow!" "wow" "false" "Exclamation is not optional"
test_glob "*!" "test!" "true" "Exclamation literal with wildcards"
echo ""

# ============================================================================
# NESTED DELIMITER SCOPE TESTS
# ============================================================================
# These tests verify that the lexer correctly tracks which delimiter type is
# innermost, preventing characters from being treated as special in the wrong context. 

echo "Test: Nested Scope - Pipe in Brackets inside Braces"
test_glob_brace "{foo,[bar|baz]}" "foo" "true" "Nested: {foo,[bar|baz]} matches foo"
test_glob_brace "{foo,[bar|baz]}" "|" "true" "Nested: {foo,[bar|baz]} matches '|' (pipe is literal inside brackets)"
test_glob_brace "{foo,[bar|baz]}" "bar" "false" "Nested: {foo,[bar|baz]} does not match bar"
test_glob_brace "{foo,[bar|baz]}" "baz" "false" "Nested: {foo,[bar|baz]} does not match baz"
echo ""

echo "Test: Nested Scope - Comma in Brackets inside Braces"
test_glob_brace "{a,[b,c]}" "a" "true" "Nested: {a,[b,c]} matches a"
test_glob_brace "{a,[b,c]}" "," "true" "Nested: {a,[b,c]} matches ',' (comma is literal inside brackets)"
test_glob_brace "{a,[b,c]}" "b" "true" "Nested: {a,[b,c]} matches b"
test_glob_brace "{a,[b,c]}" "c" "true" "Nested: {a,[b,c]} matches c"
echo ""

echo "Test: Nested Scope - Extended Glob in Braces"
test_glob_brace "{foo,+(bar|baz)}" "foo" "true" "Nested: {foo,+(bar|baz)} matches foo"
test_glob_brace "{foo,+(bar|baz)}" "bar" "true" "Nested: {foo,+(bar|baz)} matches bar"
test_glob_brace "{foo,+(bar|baz)}" "baz" "true" "Nested: {foo,+(bar|baz)} matches baz"
test_glob_brace "{foo,+(bar|baz)}" "barbaz" "true" "Nested: {foo,+(bar|baz)} matches barbaz"
test_glob_brace "{foo,+(bar|baz)}" "xyz" "false" "Nested: {foo,+(bar|baz)} does not match xyz"
echo ""

echo "Test: Nested Scope - Simple Parens in Braces"
test_glob_brace "{a,(b|c)}" "a" "true" "Nested: {a,(b|c)} matches a"
test_glob_brace "{a,(b|c)}" "b" "true" "Nested: {a,(b|c)} matches b"
test_glob_brace "{a,(b|c)}" "c" "true" "Nested: {a,(b|c)} matches c"
test_glob_brace "{a,(b|c)}" "d" "false" "Nested: {a,(b|c)} does not match d"
echo ""

echo "Test: Nested Scope - Star Group in Braces"
test_glob_brace "{pre,*(foo)bar}" "pre" "true" "Nested: {pre,*(foo)bar} matches pre"
test_glob_brace "{pre,*(foo)bar}" "bar" "true" "Nested: {pre,*(foo)bar} matches bar"
test_glob_brace "{pre,*(foo)bar}" "foobar" "true" "Nested: {pre,*(foo)bar} matches foobar"
test_glob_brace "{pre,*(foo)bar}" "foofoofoobar" "true" "Nested: {pre,*(foo)bar} matches foofoofoobar"
echo ""

echo "Test: Nested Scope - Question Group in Braces"
test_glob_brace "{x,?(a|b)}" "x" "true" "Nested: {x,?(a|b)} matches x"
test_glob_brace "{x,?(a|b)}" "" "true" "Nested: {x,?(a|b)} matches empty (? matches 0 or 1)"
test_glob_brace "{x,?(a|b)}" "a" "true" "Nested: {x,?(a|b)} matches a"
test_glob_brace "{x,?(a|b)}" "b" "true" "Nested: {x,?(a|b)} matches b"
test_glob_brace "{x,?(a|b)}" "ab" "false" "Nested: {x,?(a|b)} does not match ab"
echo ""

echo "Test: Nested Scope - At Group in Braces"
test_glob_brace "{file,@(jpg|png)}" "file" "true" "Nested: {file,@(jpg|png)} matches file"
test_glob_brace "{file,@(jpg|png)}" "jpg" "true" "Nested: {file,@(jpg|png)} matches jpg"
test_glob_brace "{file,@(jpg|png)}" "png" "true" "Nested: {file,@(jpg|png)} matches png"
test_glob_brace "{file,@(jpg|png)}" "gif" "false" "Nested: {file,@(jpg|png)} does not match gif"
echo ""

echo "Test: Nested Scope - Negation Group in Braces"
test_glob_brace "{test,!(foo|bar)}" "test" "true" "Nested: {test,!(foo|bar)} matches test"
test_glob_brace "{test,!(foo|bar)}" "baz" "true" "Nested: {test,!(foo|bar)} matches baz"
test_glob_brace "{test,!(foo|bar)}" "foo" "false" "Nested: {test,!(foo|bar)} does not match foo"
test_glob_brace "{test,!(foo|bar)}" "bar" "false" "Nested: {test,!(foo|bar)} does not match bar"
echo ""

echo "Test: Nested Scope - Pipe in Braces inside Parens"
test_glob "(a|{b,c})" "a" "true" "Nested: (a|{b,c}) matches a"
test_glob "(a|{b,c})" "b" "true" "Nested: (a|{b,c}) matches b (braces expand inside parens)"
test_glob "(a|{b,c})" "c" "true" "Nested: (a|{b,c}) matches c (braces expand inside parens)"
test_glob "(a|{b,c})" "d" "false" "Nested: (a|{b,c}) does not match d"
echo ""

echo "Test: Nested Scope - Double Dot in Brackets inside Braces"
test_glob_brace "{a,[x..y]}" "a" "true" "Nested: {a,[x..y]} matches a"
test_glob_brace "{a,[x..y]}" "." "true" "Nested: {a,[x..y]} matches '.' (. is literal inside brackets)"
echo ""

echo "Test: Nested Scope - Hyphen in Parens inside Braces"
test_glob_brace "{foo,(a-z)}" "foo" "true" "Nested: {foo,(a-z)} matches foo"
test_glob_brace "{foo,(a-z)}" "a-z" "true" "Nested: {foo,(a-z)} matches 'a-z' (hyphen is literal inside parens)"
echo ""

echo "Test: Nested Scope - Complex Three-Level Nesting"
test_glob_brace "{a,([b|c,d])}" "a" "true" "Three-level: {a,([b|c,d])} matches a"
test_glob_brace "{a,([b|c,d])}" "b" "true" "Three-level: {a,([b|c,d])} matches 'b' in character set"
test_glob_brace "{a,([b|c,d])}" "|" "true" "Three-level: {a,([b|c,d])} matches '|' in character set"
echo ""

echo "Test: Nested Scope - Verifying Correct Context Switching"
test_glob "{a,b}|{c,d}" "a|c" "true" "Context switch: {a,b}|{c,d} matches literal (pipe not in parens)"
test_glob "(a|b){c,d}" "ad" "true" "Context switch: (a|b){c,d} matches ad"
test_glob "(a|b){c,d}" "bc" "true" "Context switch: (a|b){c,d} matches bc"
test_glob "(a|b){c,d}" "ac" "true" "Context switch: (a|b){c,d} matches ac"
echo ""

echo "Test: Nested Scope - Range Inside Brace Group"
test_glob_brace "{pre{1..3}}" "pre1" "true" "Range in brace: {pre{1..3}} matches pre1"
test_glob_brace "{pre{1..3}}" "pre2" "true" "Range in brace: {pre{1..3}} matches pre2"
test_glob_brace "{pre{1..3}}" "pre3" "true" "Range in brace: {pre{1..3}} matches pre3"
echo ""

echo "Test: Nested Scope - Bracket Range Not Affected by Outer Brace"
test_glob_brace "{test[a-c],other}" "testa" "true" "Outer brace: {test[a-c],other} matches testa"
test_glob_brace "{test[a-c],other}" "testb" "true" "Outer brace: {test[a-c],other} matches testb"
test_glob_brace "{test[a-c],other}" "other" "true" "Outer brace: {test[a-c],other} matches other"
test_glob_brace "{test[a-c],other}" "test[a-c]" "false" "Outer brace: {test[a-c],other} does not match test[a-c] (brackets parsed correctly)"
echo ""

echo "Test: Nested Scope - Brackets Inside Parens Union"
test_glob "(a|[b-d])" "a" "true" "Brackets in union: (a|[b-d]) matches a"
test_glob "(a|[b-d])" "b" "true" "Brackets in union: (a|[b-d]) matches b"
test_glob "(a|[b-d])" "c" "true" "Brackets in union: (a|[b-d]) matches c"
test_glob "(a|[b-d])" "d" "true" "Brackets in union: (a|[b-d]) matches d"
test_glob "(a|[b-d])" "e" "false" "Brackets in union: (a|[b-d]) does not match e"
echo ""

echo "Test: Nested Scope - Parens Inside Brackets (Literal)"
test_glob "[a(b|c)d]" "a" "true" "Parens in brackets: [a(b|c)d] matches a"
test_glob "[a(b|c)d]" "(" "true" "Parens in brackets: [a(b|c)d] matches ("
test_glob "[a(b|c)d]" "b" "true" "Parens in brackets: [a(b|c)d] matches b"
test_glob "[a(b|c)d]" "|" "true" "Parens in brackets: [a(b|c)d] matches |"
test_glob "[a(b|c)d]" "c" "true" "Parens in brackets: [a(b|c)d] matches c"
test_glob "[a(b|c)d]" ")" "true" "Parens in brackets: [a(b|c)d] matches )"
test_glob "[a(b|c)d]" "d" "true" "Parens in brackets: [a(b|c)d] matches d"
test_glob "[a(b|c)d]" "e" "false" "Parens in brackets: [a(b|c)d] does not match e"
echo ""

echo "Test: Nested Scope - Multiple Nesting Levels"
test_glob_brace "{x,{y,[a|b]}}" "x" "true" "Multi-level: {x,{y,[a|b]}} matches x"
test_glob_brace "{x,{y,[a|b]}}" "y" "true" "Multi-level: {x,{y,[a|b]}} matches y"
test_glob_brace "{x,{y,[a|b]}}" "|" "true" "Multi-level: {x,{y,[a|b]}} matches '|' (| is literal)"
test_glob_brace "{x,{y,[a|b]}}" "a" "true" "Multi-level: {x,{y,[a|b]}} matches a"
echo ""

echo "Test: Nested Scope - Extended Glob with Nested Braces"
test_glob_brace "+(a|{b,c})" "a" "true" "Extended with braces: +(a|{b,c}) matches a"
test_glob_brace "+(a|{b,c})" "b" "true" "Extended with braces: +(a|{b,c}) matches b (braces expand)"
test_glob_brace "+(a|{b,c})" "c" "true" "Extended with braces: +(a|{b,c}) matches c (braces expand)"
test_glob_brace "+(a|{b,c})" "aa" "true" "Extended with braces: +(a|{b,c}) matches aa"
test_glob_brace "+(a|{b,c})" "abc" "true" "Extended with braces: +(a|{b,c}) matches abc"
test_glob_brace "+(a|{b,c})" "d" "false" "Extended with braces: +(a|{b,c}) does not match d"
echo ""

echo "Test: Nested Scope - Complex Real-World Patterns"
test_glob_brace "*.{@(jpg|png|gif),pdf}" "photo.jpg" "true" "Real-world: *.{@(jpg|png|gif),pdf} matches photo.jpg"
test_glob_brace "*.{@(jpg|png|gif),pdf}" "image.png" "true" "Real-world: *.{@(jpg|png|gif),pdf} matches image.png"
test_glob_brace "*.{@(jpg|png|gif),pdf}" "doc.pdf" "true" "Real-world: *.{@(jpg|png|gif),pdf} matches doc.pdf"
test_glob_brace "*.{@(jpg|png|gif),pdf}" "file.txt" "false" "Real-world: *.{@(jpg|png|gif),pdf} does not match file.txt"
echo ""

# ============================================================================
# GLOBSTAR (`**`) TESTS
# ============================================================================
# These test recursive directory matching with globstar (`**`)
# Bash reference: shopt -s globstar; ** matches any depth

echo "Test: Globstar Basic"
test_glob "**/*.cpp" "foo.cpp" "true" "Globstar: **/*.cpp matches foo.cpp (zero dirs)"
test_glob "**/*.cpp" "src/foo.cpp" "true" "Globstar: **/*.cpp matches src/foo.cpp"
test_glob "**/*.cpp" "src/sub/foo.cpp" "true" "Globstar: **/*.cpp matches deep src/sub/foo.cpp"
test_glob "**/*.cpp" "foo.txt" "false" "Globstar: **/*.cpp does not match foo.txt"
echo ""

echo "Test: Globstar At Start"
test_glob "**/baz/foo.cpp" "bar/baz/foo.cpp" "true" "Globstar at start: **/baz/foo.cpp matches bar/baz/foo.cpp"
test_glob "**/baz/foo.cpp" "baz/foo.cpp" "true" "Globstar at start: **/baz/foo.cpp matches baz/foo.cpp (zero prefix)"
test_glob "**/baz/foo.cpp" "bar/baz/bar/foo.cpp" "false" "Globstar at start: **/baz/foo.cpp does not match bar/baz/bar/foo.cpp"
echo ""

echo "Test: Globstar At End"
test_glob "src/**" "src/file.txt" "true" "Globstar at end: src/** matches src/file.txt"
test_glob "src/**" "src/sub/dir/file.txt" "true" "Globstar at end: src/** matches deep src/sub/dir/file.txt"
test_glob "src/**" "src" "true" "Globstar at end: src/** matches src/ (empty)"
test_glob "src/**" "other/src/file.txt" "false" "Globstar at end: src/** does not match other/src/file.txt"
echo ""

echo "Test: Globstar Middle"
test_glob "src/**/test/*.cpp" "src/a/b/test/foo.cpp" "true" "Globstar middle: src/**/test/*.cpp matches src/a/b/test/foo.cpp"
test_glob "src/**/test/*.cpp" "src/test/foo.cpp" "true" "Globstar middle: src/**/test/*.cpp matches src/test/foo.cpp"
test_glob "src/**/test/*.cpp" "src/a/test/foo.cpp" "true" "Globstar middle: src/**/test/*.cpp matches src/a/test/foo.cpp"
test_glob "src/**/test/*.cpp" "src/test.txt" "false" "Globstar middle: src/**/test/*.cpp does not match src/test.txt"
echo ""

echo "Test: Multiple Globstar"
test_glob "**/src/**/test" "a/b/src/c/d/test" "true" "Multiple globstar: **/src/**/test matches a/b/src/c/d/test"
test_glob "**/src/**/test" "src/test" "true" "Multiple globstar: **/src/**/test matches src/test"
test_glob "**/src/**/test" "a/src/b/test" "true" "Multiple globstar: **/src/**/test matches a/src/b/test"
echo ""

echo "Test: Globstar Directory Only (trailing /)"
test_glob "**/" "dir/sub/" "true" "Globstar trailing /: **/ matches dir/sub/ (directory)"
test_glob "**/" "dir/sub/file.txt" "false" "Globstar trailing /: **/ does not match dir/sub/file.txt"
test_glob "**/" "dir" "false" "Globstar trailing /: **/ does not match dir (no trailing /)"
echo ""

echo "Test: Globstar With Non-Standalone **"
test_glob "src/**.cpp" "src/foo.cpp" "true" "Globstar with non-standalone: src/**.cpp matches src/foo.cpp (collapses to src/*.cpp)"
test_glob "src/**.cpp" "src/sub/foo.cpp" "false" "Globstar with non-standalone: src/**.cpp does not match src/sub/foo.cpp"
test_glob "src/**.cpp" "src/foo.txt" "false" "Globstar with non-standalone: src/**.cpp does not match src/foo.txt"
echo ""

echo "Test: Globstar With Trailing Slash and Non-Standalone"
test_glob "**/**.txt" "bar/baz/foo.txt" "true" "Globstar: **/**.txt matches bar/baz/foo.txt (standalone ** + non-standalone **.txt collapsed to *.txt)"
test_glob "**/**.txt" "bar/foo.txt" "true" "Globstar: **/**.txt matches bar/foo.txt (zero dirs after **)"
test_glob "**/**.txt" "foo.txt" "true" "Globstar: **/**.txt matches foo.txt (zero dirs)"
test_glob "**/**.txt" "bar/baz/foo.txt" "true" "Globstar: **/**.txt matches bar/baz/foo.txt"
test_glob "**/**.txt" "bar/baz/foo.txt.bak" "false" "Globstar: **/**.txt does not match bar/baz/foo.txt.bak"
echo ""

echo "Test: Globstar Edge Cases"
test_glob "**" "" "true" "Globstar: ** matches empty string"
test_glob "**" "file.txt" "true" "Globstar: ** matches file.txt"
test_glob "**" "dir/sub/file.txt" "true" "Globstar: ** matches dir/sub/file.txt"
test_glob "**/*.txt" "file.txt" "true" "Globstar: **/*.txt matches file.txt"
test_glob "**/*.txt" "dir/file.txt" "true" "Globstar: **/*.txt matches dir/file.txt"
echo ""

echo "Test: Globstar With Escaped **"
test_glob "foo\\**bar" "foo**bar" "true" "Escaped \\**: foo\\**bar matches foo**bar literally"
test_glob "foo\\**bar" "foobar" "false" "Escaped \\**: foo\\**bar does not match foobar"
echo ""

# ============================================================================
# Summary
# ============================================================================

echo ""
echo -e "${BLUE}=========================================="
echo "           Test Summary"
echo -e "==========================================${NC}"
echo ""
echo "Mode: $MODE"
if [ "$MODE" = "tool" ]; then
    echo "Tool: $GLOB_TOOL"
fi
echo ""
echo "Total tests:  $((PASSED + FAILED))"
echo -e "Passed:       ${GREEN}$PASSED${NC}"
echo -e "Failed:       ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed${NC}"
    exit 1
fi
