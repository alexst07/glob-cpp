#ifndef GLOB_CPP_H
#define GLOB_CPP_H

#include <string>
#include <tuple>
#include <vector>
#include <memory>

class Automata;

class Error: public std::exception {
 public:
  Error(const std::string& msg): msg_{msg} {}

  const char* what() const throw() override {
    return msg_.c_str();
  }
 private:
  std::string msg_;
};

enum class StateType {
  MATCH,
  FAIL,
  CHAR,
  QUESTION,
  MULT,
  SET,
  GROUP,
  UNION,
};

class State {
 public:
  State(StateType type, Automata& states)
    : type_{type}
    , states_{states}{}

  virtual bool Check(const std::string& str, size_t pos) const = 0;

  virtual std::tuple<size_t, size_t>
  Next(const std::string& str, size_t pos) = 0;

  StateType Type() const {
    return type_;
  }

  const Automata& GetAutomata() const {
    return states_;
  }

  State& AddNextState(size_t state_pos) {
    next_states_.push_back(state_pos);
    return *this;
  }

  const std::vector<size_t>& GetNextStates() const {
    return next_states_;
  }

 private:
  StateType type_;
  Automata& states_;
  std::vector<size_t> next_states_;
};

class StateFail : public State {
 public:
  StateFail(Automata& states)
    : State(StateType::FAIL, states){}

  bool Check(const std::string& str, size_t pos) const override {
    return false;
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    return std::tuple<size_t, size_t>(0, ++pos);
  }
};

class StateMatch : public State {
 public:
  StateMatch(Automata& states)
    : State(StateType::MATCH, states){}

  bool Check(const std::string& str, size_t pos) const override {
    return true;
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    return std::tuple<size_t, size_t>(0, ++pos);
  }
};

class Automata {
 public:
  Automata() = default;

  Automata(const Automata&) = delete;

  Automata& operator=(const Automata& automata) = delete;

  Automata(Automata&& automata)
    : states_{std::move(automata.states_)}
    , match_state_{automata.match_state_}
    , fail_state_{std::exchange(automata.fail_state_, 0)}
    , start_state_{std::exchange(automata.start_state_, 0)} {}

  Automata& operator=(Automata&& automata) {
    states_ = std::move(automata.states_);
    match_state_ = automata.match_state_;
    fail_state_ = automata.fail_state_;
    start_state_ = automata.start_state_;

    return *this;
  }

  const State& GetState(size_t pos) const {
    return *states_[pos];
  }

  State& GetState(size_t pos) {
    return *states_[pos];
  }

  size_t FailState() const {
    return fail_state_;
  }

  Automata& SetFailState(size_t state_pos) {
    fail_state_ = state_pos;
    return *this;
  }

  Automata& SetMatchState(size_t state_pos) {
    match_state_ = state_pos;
    return *this;
  }

  size_t GetNumStates() const {
    return states_.size();
  }

  std::tuple<bool, size_t> Exec(const std::string str,
      bool comp_end = true) const {
    size_t state_pos = 0;
    size_t str_pos = 0;

    // run the state vector until state reaches fail or match state, or
    // until the string is all consumed
    while (state_pos != fail_state_ && state_pos != match_state_
           && str_pos < str.length()) {
      std::tie(state_pos, str_pos) = states_[state_pos]->Next(str, str_pos);
    }

    // if comp_end is true it matches only if the automata reached the end of
    // the string
    if (comp_end) {
      if ((state_pos == match_state_) && (str_pos == str.length())) {
        return std::tuple<bool, size_t>(state_pos == match_state_, str_pos);
      }

      return std::tuple<bool, size_t>(false, str_pos);
    } else {
      // if comp_end is false, compare only if the states reached the
      // match state
      return std::tuple<bool, size_t>(state_pos == match_state_, str_pos);
    }
  }

  template<class T, typename... Args>
  size_t NewState(Args&&... args) {
    size_t state_pos = states_.size();
    auto state = std::unique_ptr<State>(new T(*this,
        std::forward<Args>(args)...));

    states_.push_back(std::move(state));
    return state_pos;
  }

  size_t fail_state_;
 private:
  std::vector<std::unique_ptr<State>> states_;
  size_t match_state_;

  size_t start_state_;
};

class StateChar : public State {
 public:
  StateChar(Automata& states, char c)
    : State(StateType::CHAR, states)
    , c_{c}{}

  bool Check(const std::string& str, size_t pos) const override {
    return(c_ == str[pos]);
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    if (c_ == str[pos]) {
      return std::tuple<size_t, size_t>(GetNextStates()[0], pos + 1);
    }

    return std::tuple<size_t, size_t>(GetAutomata().FailState(), pos + 1);
  }
 private:
  char c_;
};

class StateAny : public State {
 public:
  StateAny(Automata& states)
    : State(StateType::QUESTION, states){}

  bool Check(const std::string& str, size_t pos) const override {
    // as it match any char, it is always trye
    return true;
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    // state any always match with any char
    return std::tuple<size_t, size_t>(GetNextStates()[0], pos + 1);
  }
};

class StateStar : public State {
 public:
  StateStar(Automata& states)
    : State(StateType::MULT, states){}

  bool Check(const std::string& str, size_t pos) const override {
    // as it match any char, it is always trye
    return true;
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    // next state vector from StateStar has two elements, the element 0 points
    // to the same state, and the element points to next state if the
    // conditions is satisfied
    if (GetAutomata().GetState(GetNextStates()[1]).Type() == StateType::MATCH) {
      // this case occurs when star is in the end of the glob, so the pos is
      // the end of the string, because all string is consumed
      return std::tuple<size_t, size_t>(GetNextStates()[1], str.length());
    }

    bool res = GetAutomata().GetState(GetNextStates()[1]).Check(str, pos);
    // if the next state is satisfied goes to next state
    if (res) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    // while the next state check is false, the string is consumed by star state
    return std::tuple<size_t, size_t>(GetNextStates()[0], pos + 1);
  }
};

class SetItem {
 public:
  SetItem() = default;

  virtual bool Check(char c) const = 0;
};

class SetItemChar: public SetItem {
 public:
  SetItemChar(char c): c_{c} {}

  bool Check(char c) const override {
    return c == c_;
  }
 private:
  char c_;
};

class SetItemRange: public SetItem {
 public:
  SetItemRange(char start, char end)
    : start_{start < end? start : end}
    , end_{start < end? end : start} {}

  bool Check(char c) const override {
    return (c >= start_) && (c <= end_);
  }
 private:
  char start_;
  char end_;
};

class StateSet : public State {
 public:
  StateSet(Automata& states, std::vector<std::unique_ptr<SetItem>> items,
      bool neg = false)
    : State(StateType::SET, states)
    , items_{std::move(items)}
    , neg_{neg} {}

  bool SetCheck(const std::string& str, size_t pos) const {
    for (auto& item : items_) {
      // if any item match, then the set match with char
      if (item.get()->Check(str[pos])) {
        return true;
      }
    }

    return false;
  }

  bool Check(const std::string& str, size_t pos) const override {
    if (neg_) {
      return !SetCheck(str, pos);
    }

    return SetCheck(str, pos);
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    if (Check(str, pos)) {
      return std::tuple<size_t, size_t>(GetNextStates()[0], pos + 1);
    }

    return std::tuple<size_t, size_t>(GetAutomata().FailState(), pos + 1);
  }
 private:
  std::vector<std::unique_ptr<SetItem>> items_;
  bool neg_;
};

class StateGroup: public State {
 public:
  enum class Type {
    BASIC,
    ANY,
    STAR,
    PLUS,
    NEG,
    AT
  };

  StateGroup(Automata& states, Type type,
      std::vector<std::unique_ptr<Automata>>&& automatas)
    : State(StateType::QUESTION, states)
    , type_{type}
    , automatas_{std::move(automatas)}
    , match_one_{false} {}

  std::tuple<bool, size_t> BasicCheck(const std::string& str,
      size_t pos) const {
    std::string str_part = str.substr(pos);
    bool r;
    size_t str_pos;

    // each automata is a part of a union of the group, in basic check,
    // we want find only if any automata is true
    for (auto& automata : automatas_) {
      std::tie(r, str_pos) = automata->Exec(str_part, false);
      if (r) {
        return std::tuple<bool, size_t>(r, pos + str_pos);
      }
    }

    return std::tuple<bool, size_t>(false, pos + str_pos);
  }

  bool Check(const std::string& str, size_t pos) const override {
    switch (type_) {
      case Type::BASIC:
      case Type::AT: {
        bool r;
        std::tie(r, std::ignore) = BasicCheck(str, pos);
        return r;
        break;
      }

      case Type::ANY: {
        return true;
        break;
      }

      case Type::STAR: {
        return true;
        break;
      }

      case Type::PLUS: {
        bool r;
        std::tie(r, std::ignore) = BasicCheck(str, pos);
        return r;
        break;
      }

      case Type::NEG: {
        bool r;
        std::tie(r, std::ignore) = BasicCheck(str, pos);
        return !r;
        break;
      }

      default:
        return false;
        break;
    }
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    // STATE 1 -> is the next state
    // STATE 0 -> is the same state
    switch (type_) {
      case Type::BASIC:
      case Type::AT: {
        return NextBasic(str, pos);
        break;
      }

      case Type::ANY: {
        return NextAny(str, pos);
        break;
      }

      case Type::STAR: {
        return NextStar(str, pos);
        break;
      }

      case Type::PLUS: {
        return NextPlus(str, pos);
        break;
      }

      case Type::NEG: {
        return NextNeg(str, pos);
        break;
      }
    }
  }

  std::tuple<size_t, size_t> NextNeg(const std::string& str, size_t pos) {
    bool r;
    size_t new_pos;
    std::tie(r, new_pos) = BasicCheck(str, pos);
    if (r) {
      return std::tuple<size_t, size_t>(GetAutomata().FailState(), new_pos);
    }

    return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
  }

  std::tuple<size_t, size_t> NextBasic(const std::string& str, size_t pos) {
    bool r;
    size_t new_pos;
    std::tie(r, new_pos) = BasicCheck(str, pos);
    if (r) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], new_pos);
    }

    return std::tuple<size_t, size_t>(GetAutomata().FailState(), new_pos);
  }

  std::tuple<size_t, size_t> NextAny(const std::string& str, size_t pos) {
    bool res = GetAutomata().GetState(GetNextStates()[1]).Check(str, pos);
    if (res) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    bool r;
    size_t new_pos;
    std::tie(r, new_pos) = BasicCheck(str, pos);
    if (r) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], new_pos);
    }

    if (GetAutomata().GetState(GetNextStates()[1]).Type() == StateType::MATCH) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
  }

  std::tuple<size_t, size_t> NextStar(const std::string& str, size_t pos) {
    bool res = GetAutomata().GetState(GetNextStates()[1]).Check(str, pos);
    if (res) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    bool r;
    size_t new_pos;
    std::tie(r, new_pos) = BasicCheck(str, pos);
    if (r) {
      if (GetAutomata().GetState(GetNextStates()[1]).Type() == StateType::MATCH
          && new_pos == str.length()) {
        return std::tuple<size_t, size_t>(GetNextStates()[1], new_pos);
      } else {
        return std::tuple<size_t, size_t>(GetNextStates()[0], new_pos);
      }
    }

    if (GetAutomata().GetState(GetNextStates()[1]).Type() == StateType::MATCH) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
  }

  std::tuple<size_t, size_t> NextPlus(const std::string& str, size_t pos) {
    // case where the next state matches and the group already matched
    // one time -> goes to next state
    bool res = GetAutomata().GetState(GetNextStates()[1]).Check(str, pos);
    if (res && match_one_) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    bool r;
    size_t new_pos;
    std::tie(r, new_pos) = BasicCheck(str, pos);
    if (r) {
      match_one_ = true;

      // if it matches and the string reached at the end, and the next
      // state is the match state, goes to next state to avoid state mistake
      if (GetAutomata().GetState(GetNextStates()[1]).Type() == StateType::MATCH
          && new_pos == str.length()) {
        return std::tuple<size_t, size_t>(GetNextStates()[1], new_pos);
      } else {
        return std::tuple<size_t, size_t>(GetNextStates()[0], new_pos);
      }
    }

    if (match_one_) {
      if (GetAutomata().GetState(GetNextStates()[1]).Type()
          == StateType::MATCH) {
        return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
      }

      bool res = GetAutomata().GetState(GetNextStates()[1]).Check(str, pos);
      // if the next state is satisfied goes to next state
      if (res) {
        return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
      }

      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    } else {
      return std::tuple<size_t, size_t>(GetAutomata().FailState(), new_pos);
    }
  }

 private:
  Type type_;
  std::vector<std::unique_ptr<Automata>> automatas_;
  bool match_one_;
};

enum class TokenKind {
  UNKNOWN = 0,
  CHAR,
  #define TOKEN(X, Y) X,
  #include "token.def"
  NUM_TOKENS
};

static const char* token_name_str[] = {
  "UNKNOWN", // UNKNOWN
  "CHAR",
#define TOKEN(X, Y) #X,
#include "token.def"
  ""
};

class Token {
 public:
  Token(TokenKind kind): kind_{kind} {}
  Token(TokenKind kind, char value): kind_{kind}, value_{value} {}
  TokenKind Kind() const {
    return kind_;
  }

  char Value() const {
    return value_;
  }

  bool operator==(TokenKind kind) {
    return kind_ == kind;
  }

  bool operator==(TokenKind kind) const {
    return kind_ == kind;
  }

  bool operator!=(TokenKind kind) {
    return kind_ != kind;
  }

  bool operator!=(TokenKind kind) const {
    return kind_ != kind;
  }
 private:
  friend std::ostream& operator<<(std::ostream& stream, const Token& token);
  TokenKind kind_;
  char value_;
};

inline std::ostream& operator<<(std::ostream& stream, const Token& token) {
  stream  << '[' << token_name_str[static_cast<int>(token.kind_)] << ']';
  return stream;
}

class Lexer {
 public:
  static const char kEndOfInput = -1;

  Lexer(const std::string& str): str_(str), pos_{0}, c_{str[0]} {}

  std::vector<Token> Scanner() {
    std::vector<Token> tokens;
    while(true) {
      switch (c_) {
        case '?': {
          Advance();
          if (c_ == '(') {
            tokens.push_back(Select(TokenKind::QUESTLPAREN));
            Advance();
          } else {
            tokens.push_back(Select(TokenKind::QUESTION));
          }
          break;
        }

        case '*': {
          Advance();
          if (c_ == '(') {
            tokens.push_back(Select(TokenKind::STARLPAREN));
            Advance();
          } else {
            tokens.push_back(Select(TokenKind::STAR));
          }
          break;
        }

        case '+': {
          Advance();
          if (c_ == '(') {
            tokens.push_back(Select(TokenKind::PLUSLPAREN));
            Advance();
          } else {
            tokens.push_back(Select(TokenKind::CHAR, '+'));
          }
          break;
        }

        case '-': {
          tokens.push_back(Select(TokenKind::SUB));
          Advance();
          break;
        }

        case '|': {
          tokens.push_back(Select(TokenKind::UNION));
          Advance();
          break;
        }

        case '@': {
          Advance();
          if (c_ == '(') {
            tokens.push_back(Select(TokenKind::ATLPAREN));
            Advance();
          } else {
            tokens.push_back(Select(TokenKind::CHAR, '@'));
          }
          break;
        }

        case '!': {
          Advance();
          if (c_ == '(') {
            tokens.push_back(Select(TokenKind::NEGLPAREN));
            Advance();
          } else {
            tokens.push_back(Select(TokenKind::CHAR, '!'));
          }
          break;
        }

        case '(': {
          tokens.push_back(Select(TokenKind::LPAREN));
          Advance();
          break;
        }

        case ')': {
          tokens.push_back(Select(TokenKind::RPAREN));
          Advance();
          break;
        }

        case '[': {
          Advance();
          if (c_ == '!') {
            tokens.push_back(Select(TokenKind::NEGLBRACKET));
            Advance();
          } else {
            tokens.push_back(Select(TokenKind::LBRACKET));
          }
          break;
        }

        case ']': {
          tokens.push_back(Select(TokenKind::RBRACKET));
          Advance();
          break;
        }

        case '\\': {
          Advance();
          if (c_ == kEndOfInput) {
            throw Error("No valid char after '\\'");
          } else if (IsSpecialChar(c_)) {
            tokens.push_back(Select(TokenKind::CHAR, c_));
            Advance();
          }
          break;
        }

        default: {
          if (c_ == kEndOfInput) {
            tokens.push_back(Select(TokenKind::EOS));
            return tokens;
          } else {
            tokens.push_back(Select(TokenKind::CHAR, c_));
            Advance();
          }
        }
      }
    }
  }

 private:
  inline Token Select(TokenKind k) {
    return Token(k);
  }

  inline Token Select(TokenKind k, char value) {
    return Token(k, value);
  }

  void Advance() {
    if (pos_ == (str_.length() - 1)) {
      c_ = kEndOfInput;
      return;
    }

    c_ = str_[++pos_];
  }

  inline bool IsSpecialChar(char c) {
    bool b = c == '?' ||
             c == '*' ||
             c == '+' ||
             c == '(' ||
             c == ')' ||
             c == '[' ||
             c == ']' ||
             c == '|' ||
             c == '!' ||
             c == '@' ||
             c == '\\';
    return b;
  }

  std::string str_;
  size_t pos_;
  char c_;
};


#define AST_NODE_LIST(V)  \
  V(CharNode)             \
  V(RangeNode)            \
  V(SetItemsNode)         \
  V(PositiveSetNode)      \
  V(NegativeSetNode)      \
  V(StarNode)             \
  V(AnyNode)              \
  V(GroupNode)            \
  V(ConcatNode)           \
  V(UnionNode)            \
  V(GlobNode)

class AstVisitor;

// declare all classes used for nodes
#define DECLARE_TYPE_CLASS(type) class type;
  AST_NODE_LIST(DECLARE_TYPE_CLASS)
#undef DECLARE_TYPE_CLASS

class AstNode {
 public:
  enum class Type {
    CHAR,
    RANGE,
    SET_ITEM,
    SET_ITEMS,
    POS_SET,
    NEG_SET,
    SET,
    STAR,
    ANY,
    GROUP,
    CONCAT_GLOB,
    UNION,
    GLOB
  };

  Type GetType() const {
    return type_;
  }

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  AstNode(Type type): type_{type} {}

 private:
  Type type_;
};

class AstVisitor {
 public:

// define all visitor methods for the nodes
#define DECLARE_VIRTUAL_FUNC(type) \
  virtual void Visit##type(type* /*node*/) {}
  AST_NODE_LIST(DECLARE_VIRTUAL_FUNC)
#undef DECLARE_VIRTUAL_FUNC
};


class CharNode: public AstNode {
 public:
  CharNode(char c): AstNode(Type::CHAR), c_{c} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCharNode(this);
  }

  char GetValue() const {
    return c_;
  }

 private:
  char c_;
};

class RangeNode: public AstNode {
 public:
  RangeNode(std::unique_ptr<AstNode>&& start, std::unique_ptr<AstNode>&& end)
    : AstNode(Type::RANGE)
    , start_{std::move(start)}
    , end_{std::move(end)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitRangeNode(this);
  }

  AstNode* GetStart() const {
    return start_.get();
  }

  AstNode* GetEnd() const {
    return end_.get();
  }

 private:
  std::unique_ptr<AstNode> start_;
  std::unique_ptr<AstNode> end_;
};

class SetItemsNode: public AstNode {
 public:
  SetItemsNode(std::vector<std::unique_ptr<AstNode>>&& items)
    : AstNode(Type::SET_ITEMS)
    , items_{std::move(items)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSetItemsNode(this);
  }

  std::vector<std::unique_ptr<AstNode>>& GetItems() {
    return items_;
  }

 private:
  std::vector<std::unique_ptr<AstNode>> items_;
};

class PositiveSetNode: public AstNode {
 public:
  PositiveSetNode(std::unique_ptr<AstNode>&& set)
    : AstNode(Type::POS_SET)
    , set_{std::move(set)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitPositiveSetNode(this);
  }

  AstNode* GetSet() {
    return set_.get();
  }

 private:
  std::unique_ptr<AstNode> set_;
};

class NegativeSetNode: public AstNode {
 public:
  NegativeSetNode(std::unique_ptr<AstNode>&& set)
    : AstNode(Type::NEG_SET)
    , set_{std::move(set)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitNegativeSetNode(this);
  }

  AstNode* GetSet() {
    return set_.get();
  }

 private:
  std::unique_ptr<AstNode> set_;
};

class StarNode: public AstNode {
 public:
  StarNode()
    : AstNode(Type::STAR) {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitStarNode(this);
  }
};

class AnyNode: public AstNode {
 public:
  AnyNode()
    : AstNode(Type::ANY) {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAnyNode(this);
  }
};

class GroupNode: public AstNode {
 public:
  enum class GroupType {
    BASIC,
    ANY,
    STAR,
    PLUS,
    NEG,
    AT
  };

  GroupNode(GroupType group_type, std::unique_ptr<AstNode>&& glob)
    : AstNode(Type::GROUP)
    , glob_{std::move(glob)}
    , group_type_{group_type} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitGroupNode(this);
  }

  AstNode* GetGlob() {
    return glob_.get();
  }

  GroupType GetGroupType() const {
    return group_type_;
  }

 private:
  std::unique_ptr<AstNode> glob_;
  GroupType group_type_;
};

class ConcatNode: public AstNode {
 public:
  ConcatNode(std::vector<std::unique_ptr<AstNode>>&& basic_glob)
    : AstNode(Type::CONCAT_GLOB)
    , basic_glob_{std::move(basic_glob)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitConcatNode(this);
  }

  std::vector<std::unique_ptr<AstNode>>& GetBasicGlobs() {
    return basic_glob_;
  }

 private:
  std::vector<std::unique_ptr<AstNode>> basic_glob_;
};

class UnionNode: public AstNode {
 public:
  UnionNode(std::vector<std::unique_ptr<AstNode>>&& items)
    : AstNode(Type::UNION)
    , items_{std::move(items)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitUnionNode(this);
  }

  std::vector<std::unique_ptr<AstNode>>& GetItems() {
    return items_;
  }

 private:
  std::vector<std::unique_ptr<AstNode>> items_;
};

class GlobNode: public AstNode {
 public:
  GlobNode(std::unique_ptr<AstNode>&& glob)
    : AstNode(Type::GLOB)
    , glob_{std::move(glob)} {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitGlobNode(this);
  }

  AstNode* GetConcat() {
    return glob_.get();
  }

 private:
  std::unique_ptr<AstNode> glob_;
};

using AstNodePtr = std::unique_ptr<AstNode>;

class Parser {
 public:
  Parser() = delete;

  Parser(std::vector<Token>&& tok_vec): tok_vec_{std::move(tok_vec)}, pos_{0} {}

  AstNodePtr GenAst() {
    return ParserGlob();
  }

 private:
  AstNodePtr ParserChar() {
    Token& tk = NextToken();
    if (tk != TokenKind::CHAR) {
      throw Error("char expected");
    }

    char c = tk.Value();
    return AstNodePtr(new CharNode(c));
  }

  AstNodePtr ParserRange() {
    AstNodePtr char_start = ParserChar();

    Token& tk = NextToken();
    if (tk != TokenKind::SUB) {
      throw Error("range expected");
    }

    AstNodePtr char_end = ParserChar();
    return AstNodePtr(
        new RangeNode(std::move(char_start), std::move(char_end)));
  }

  AstNodePtr ParserSetItem() {
    if (PeekAhead() == TokenKind::SUB) {
      return ParserRange();
    }

    return ParserChar();
  }

  AstNodePtr ParserSetItems() {
    std::vector<AstNodePtr> items;

    do {
      items.push_back(ParserSetItem());
    } while (GetToken() != TokenKind::RBRACKET);

    Advance();

    return AstNodePtr(new SetItemsNode(std::move(items)));
  }

  AstNodePtr ParserSet() {
    Token& tk = NextToken();

    if (tk == TokenKind::LBRACKET) {
      return AstNodePtr(new PositiveSetNode(ParserSetItems()));
    } else if (tk == TokenKind::NEGLBRACKET) {
      return AstNodePtr(new NegativeSetNode(ParserSetItems()));
    } else {
      throw Error("set expected");
    }
  }

  AstNodePtr ParserBasicGlob() {
    Token& tk = GetToken();

    switch (tk.Kind()) {
      case TokenKind::QUESTION:
        Advance();
        return AstNodePtr(new AnyNode());
        break;

      case TokenKind::STAR:
        Advance();
        return AstNodePtr(new StarNode());
        break;

      case TokenKind::SUB:
        Advance();
        return AstNodePtr(new CharNode('-'));
        break;

      case TokenKind::CHAR:
        return ParserChar();
        break;

      case TokenKind::LBRACKET:
      case TokenKind::NEGLBRACKET:
        return ParserSet();
        break;

      case TokenKind::LPAREN:
      case TokenKind::QUESTLPAREN:
      case TokenKind::STARLPAREN:
      case TokenKind::PLUSLPAREN:
      case TokenKind::NEGLPAREN:
      case TokenKind::ATLPAREN:
        return ParserGroup();
        break;

      default:
        throw Error("basic glob expected");
        break;
    }
  }

  AstNodePtr ParserGroup() {
    GroupNode::GroupType type;
    Token& tk = NextToken();

    switch (tk.Kind()) {
      case TokenKind::LPAREN:
        type = GroupNode::GroupType::BASIC;
        break;

      case TokenKind::QUESTLPAREN:
        type = GroupNode::GroupType::ANY;
        break;

      case TokenKind::STARLPAREN:
        type = GroupNode::GroupType::STAR;
        break;

      case TokenKind::PLUSLPAREN:
        type = GroupNode::GroupType::PLUS;
        break;

      case TokenKind::NEGLPAREN:
        type = GroupNode::GroupType::NEG;
        break;

      case TokenKind::ATLPAREN:
        type = GroupNode::GroupType::AT;
        break;

      default:
        throw Error("Not valid group");
        break;
    }

    AstNodePtr group_glob = ParserUnion();
    tk = NextToken();
    if (tk != TokenKind::RPAREN) {
      throw Error("Expected ')' at and of group");
    }

    return AstNodePtr(new GroupNode(type, std::move(group_glob)));
  }

  AstNodePtr ParserConcat() {
    auto check_end = [&]() -> bool {
      Token& tk = GetToken();

      switch (tk.Kind()) {
        case TokenKind::EOS:
        case TokenKind::RPAREN:
        case TokenKind::UNION:
          return true;
          break;

        default:
          return false;
          break;
      }
    };

    std::vector<AstNodePtr> parts;

    while (!check_end()) {
      parts.push_back(ParserBasicGlob());
    }

    return AstNodePtr(new ConcatNode(std::move(parts)));
  }

  AstNodePtr ParserUnion() {
    std::vector<AstNodePtr> items;
    items.push_back(ParserConcat());

    while (GetToken() == TokenKind::UNION) {
      Advance();
      items.push_back(ParserConcat());
    }

    return AstNodePtr(new UnionNode(std::move(items)));
  }

  AstNodePtr ParserGlob() {
    AstNodePtr glob = ParserConcat();

    if (GetToken() != TokenKind::EOS) {
      throw Error("Expected the end of glob");
    }

    return AstNodePtr(new GlobNode(std::move(glob)));
  }

  inline const Token& GetToken() const {
    return tok_vec_.at(pos_);
  }

  inline Token& GetToken() {
    return tok_vec_.at(pos_);
  }

  inline const Token& PeekAhead() const {
    if (pos_ >= (tok_vec_.size() - 1))
      return tok_vec_.back();

    return tok_vec_.at(pos_ + 1);
  }

  inline Token& NextToken() {
    if (pos_ >= (tok_vec_.size() - 1))
      return tok_vec_.back();

    Token& tk = tok_vec_.at(pos_);
    pos_++;
    return tk;
  }

  inline bool Advance() {
    if (pos_ == tok_vec_.size() - 1)
      return false;

    ++pos_;
    return true;
  }

  inline size_t Size() const noexcept {
    return tok_vec_.size();
  }

  std::vector<Token> tok_vec_;
  size_t pos_;
};

class AstConsumer {
 public:
  AstConsumer() = default;

  void GenAutomata(AstNode* root_node, Automata& automata) {
    AstNode* concat_node = static_cast<GlobNode*>(root_node)->GetConcat();
    ExecConcat(concat_node, automata);

    size_t match_state = automata.NewState<StateMatch>();
    automata.GetState(preview_state_).AddNextState(match_state);
    automata.SetMatchState(match_state);

    size_t fail_state = automata.NewState<StateFail>();
    automata.SetFailState(fail_state);
  }

 private:
  void ExecConcat(AstNode* node, Automata& automata) {
    ConcatNode* concat_node = static_cast<ConcatNode*>(node);
    std::vector<AstNodePtr>& basic_globs = concat_node->GetBasicGlobs();

    for (auto& basic_glob : basic_globs) {
      ExecBasicGlob(basic_glob.get(), automata);
    }
  }

  void ExecBasicGlob(AstNode* node, Automata& automata) {
    switch (node->GetType()) {
      case AstNode::Type::CHAR:
        ExecChar(node, automata);
        break;

      case AstNode::Type::ANY:
        ExecAny(node, automata);
        break;

      case AstNode::Type::STAR:
        ExecStar(node, automata);
        break;

      case AstNode::Type::POS_SET:
        ExecPositiveSet(node, automata);
        break;

      case AstNode::Type::NEG_SET:
        ExecNegativeSet(node, automata);
        break;

      case AstNode::Type::GROUP:
        ExecGroup(node, automata);
        break;

      default:
        break;
    }
  }

  void ExecChar(AstNode* node, Automata& automata) {
    CharNode* char_node = static_cast<CharNode*>(node);
    char c = char_node->GetValue();
    NewState<StateChar>(automata, c);
  }

  void ExecAny(AstNode* node, Automata& automata) {
    NewState<StateAny>(automata);
  }

  void ExecStar(AstNode* node, Automata& automata) {
    NewState<StateStar>(automata);
    automata.GetState(current_state_).AddNextState(current_state_);
  }

  void ExecPositiveSet(AstNode* node, Automata& automata) {
    PositiveSetNode* pos_set_node = static_cast<PositiveSetNode*>(node);
    auto items = ProcessSetItems(pos_set_node->GetSet());
    NewState<StateSet>(automata, std::move(items));
  }

  void ExecNegativeSet(AstNode* node, Automata& automata) {
    NegativeSetNode* pos_set_node = static_cast<NegativeSetNode*>(node);
    auto items = ProcessSetItems(pos_set_node->GetSet());
    NewState<StateSet>(automata, std::move(items), /*neg*/true);
  }

  std::vector<std::unique_ptr<SetItem>> ProcessSetItems(AstNode* node) {
    SetItemsNode* set_node = static_cast<SetItemsNode*>(node);
    std::vector<std::unique_ptr<SetItem>> vec;
    auto& items = set_node->GetItems();
    for (auto& item : items) {
      vec.push_back(ProcessSetItem(item.get()));
    }

    return vec;
  }

  std::unique_ptr<SetItem> ProcessSetItem(AstNode* node) {
    if (node->GetType() == AstNode::Type::CHAR) {
      CharNode* char_node = static_cast<CharNode*>(node);
      char c = char_node->GetValue();
      return std::unique_ptr<SetItem>(new SetItemChar(c));
    } else if (node->GetType() == AstNode::Type::RANGE) {
      RangeNode* range_node = static_cast<RangeNode*>(node);
      CharNode* start_node = static_cast<CharNode*>(range_node->GetStart());
      CharNode* end_node = static_cast<CharNode*>(range_node->GetEnd());
      char start_char = start_node->GetValue();
      char end_char = end_node->GetValue();
      return std::unique_ptr<SetItem>(new SetItemRange(start_char, end_char));
    } else {
      throw Error("Not valid set item");
    }
  }

  void ExecGroup(AstNode* node, Automata& automata) {
    GroupNode* group_node = static_cast<GroupNode*>(node);
    AstNode* union_node = group_node->GetGlob();
    std::vector<std::unique_ptr<Automata>> automatas = ExecUnion(union_node);

    StateGroup::Type state_group_type;
    switch (group_node->GetGroupType()) {
      case GroupNode::GroupType::BASIC:
        state_group_type = StateGroup::Type::BASIC;
        break;

      case GroupNode::GroupType::ANY:
        state_group_type = StateGroup::Type::ANY;
        break;

      case GroupNode::GroupType::STAR:
        state_group_type = StateGroup::Type::STAR;
        break;

      case GroupNode::GroupType::PLUS:
        state_group_type = StateGroup::Type::PLUS;
        break;

      case GroupNode::GroupType::AT:
        state_group_type = StateGroup::Type::AT;
        break;

      case GroupNode::GroupType::NEG:
        state_group_type = StateGroup::Type::NEG;
        break;
    }

    NewState<StateGroup>(automata, state_group_type, std::move(automatas));
    automata.GetState(current_state_).AddNextState(current_state_);
  }

  std::vector<std::unique_ptr<Automata>> ExecUnion(AstNode* node) {
    UnionNode* union_node = static_cast<UnionNode*>(node);
    auto& items = union_node->GetItems();
    std::vector<std::unique_ptr<Automata>> vec_automatas;
    for (auto& item : items) {
      std::unique_ptr<Automata> automata_ptr(new Automata);
      AstConsumer ast_consumer;
      ast_consumer.ExecConcat(item.get(), *automata_ptr);

      size_t match_state = automata_ptr->NewState<StateMatch>();
      automata_ptr->GetState(ast_consumer.preview_state_)
          .AddNextState(match_state);
      automata_ptr->SetMatchState(match_state);

      size_t fail_state = automata_ptr->NewState<StateFail>();
      automata_ptr->SetFailState(fail_state);

      vec_automatas.push_back(std::move(automata_ptr));
    }

    return vec_automatas;
  }

  template<class T, typename... Args>
  void NewState(Automata& automata, Args&&... args) {
    current_state_ = automata.NewState<T>(std::forward<Args>(args)...);
    if (preview_state_ >= 0) {
      automata.GetState(preview_state_).AddNextState(current_state_);
    }
    preview_state_ = current_state_;
  }

 private:
  int preview_state_ = -1;
  size_t current_state_ = 0;
};

class Glob {
 public:
  Glob(const std::string& str) {
    Lexer l(str);
    std::vector<Token> tokens = l.Scanner();
    Parser p(std::move(tokens));
    AstNodePtr ast_ptr = p.GenAst();

    AstConsumer ast_consumer;
    ast_consumer.GenAutomata(ast_ptr.get(), automata_);
  }

  bool Exec(const std::string& str) {
    bool r;
    size_t pos;
    std::tie(r, pos) = automata_.Exec(str);
    return r;
  }

 private:
  Automata automata_;
};
class SimpleGlob {
 public:
  SimpleGlob() = default;
  void Parser(const std::string& pattern) {
    size_t pos = 0;
    int preview_state = -1;

    while(pos < pattern.length()) {
      size_t current_state = 0;
      char c = pattern[pos];
      switch (c) {
        case '?': {
          current_state = automato_.NewState<StateAny>();
          ++pos;
          break;
        }

        case '*': {
          current_state = automato_.NewState<StateStar>();
          automato_.GetState(current_state).AddNextState(current_state);
          ++pos;
          break;
        }

        default: {
          current_state = automato_.NewState<StateChar>(c);
          ++pos;
          break;
        }
      }

      if (preview_state >= 0) {
        automato_.GetState(preview_state).AddNextState(current_state);
      }
      preview_state = current_state;
    }

    size_t match_state = automato_.NewState<StateMatch>();
    automato_.GetState(preview_state).AddNextState(match_state);
    automato_.SetMatchState(match_state);

    size_t fail_state = automato_.NewState<StateFail>();
    automato_.SetFailState(fail_state);
  }

  Automata& GetAutomata() {
    return automato_;
  }
 private:
  Automata automato_;
};

#endif  // GLOB_CPP_H