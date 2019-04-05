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
  MULT
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

  const Automata& GetAutomato() const {
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

  const State& GetState(size_t pos) const {
    return *states_[pos];
  }

  State& GetState(size_t pos) {
    return *states_[pos];
  }

  const size_t FailState() const {
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

  bool Exec(const std::string str) {
    size_t state_pos = 0;
    size_t str_pos = 0;

    while (state_pos != fail_state_ && state_pos != match_state_
           && str_pos < str.length()) {
      std::tie(state_pos, str_pos) = states_[state_pos]->Next(str, str_pos);
    }

    return state_pos == match_state_;
  }

  template<class T, typename... Args>
  size_t NewState(Args&&... args) {
    size_t state_pos = states_.size();
    auto state = std::unique_ptr<State>(new T(*this,
        std::forward<Args>(args)...));

    states_.push_back(std::move(state));
    return state_pos;
  }

 private:
  std::vector<std::unique_ptr<State>> states_;
  size_t match_state_;
  size_t fail_state_;
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

    return std::tuple<size_t, size_t>(GetAutomato().FailState(), pos + 1);
  }
 private:
  char c_;
};

class StateQuestion : public State {
 public:
  StateQuestion(Automata& states)
    : State(StateType::QUESTION, states){}

  bool Check(const std::string& str, size_t pos) const override {
    return true;
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    return std::tuple<size_t, size_t>(GetNextStates()[0], pos + 1);
  }
};

class StateMult : public State {
 public:
  StateMult(Automata& states)
    : State(StateType::MULT, states){}

  bool Check(const std::string& str, size_t pos) const override {
    return true;
  }

  std::tuple<size_t, size_t> Next(const std::string& str, size_t pos) override {
    if (GetAutomato().GetState(GetNextStates()[1]).Type() == StateType::MATCH) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    bool res = GetAutomato().GetState(GetNextStates()[1]).Check(str, pos);
    if (res) {
      return std::tuple<size_t, size_t>(GetNextStates()[1], pos);
    }

    return std::tuple<size_t, size_t>(GetNextStates()[0], pos + 1);
  }
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
          tokens.push_back(Select(TokenKind::QUESTION));
          Advance();
          break;
        }

        case '*': {
          tokens.push_back(Select(TokenKind::STAR));
          Advance();
          break;
        }

        case '+': {
          tokens.push_back(Select(TokenKind::PLUS));
          Advance();
          break;
        }

        case '-': {
          tokens.push_back(Select(TokenKind::SUB));
          Advance();
          break;
        }

        case '@': {
          tokens.push_back(Select(TokenKind::AT));
          Advance();
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
          if (c_ == '^') {
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
    bool b = c != '?' &&
             c != '*' &&
             c != '+' &&
             c != '^' &&
             c != '(' &&
             c != ')' &&
             c != '[' &&
             c != ']' &&
             c != '|' &&
             c != '!' &&
             c != '@' &&
             c != '\\';
    return b;
  }

  std::string str_;
  size_t pos_;
  char c_;
};


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
    BASIC_GLOB,
    CONCAT_GLOB,
    UNION,
    GLOB
  };

  Type GetType() const {
    return type_;
  }

 protected:
  AstNode(Type type): type_{type} {}

 private:
  Type type_;
};

class CharNode: public AstNode {
 public:
  CharNode(char c): AstNode(Type::CHAR), c_{c} {}

  char GetValue() const {
    return c_;
  }

 private:
  char c_;
};

class RangeNode: public AstNode {
 public:
  RangeNode(char start, char end)
    : AstNode(Type::RANGE)
    , start_{start_}
    , end_{end} {}

  char GetStart() const {
    return start_;
  }

  char GetEnd() const {
    return end_;
  }

 private:
  char start_;
  char end_;
};

class SetItemNode: public AstNode {
 public:
  SetItemNode(std::unique_ptr<AstNode>&& item)
    : AstNode(Type::SET_ITEM)
    , item_{std::move(item)} {}

  AstNode* GetItem() {
    return item_.get();
  }

 private:
  std::unique_ptr<AstNode> item_;
};

class SetItemsNode: public AstNode {
 public:
  SetItemsNode(std::vector<std::unique_ptr<AstNode>>&& items)
    : AstNode(Type::SET_ITEMS)
    , items_{std::move(items)} {}

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
};

class AnyNode: public AstNode {
 public:
  AnyNode()
    : AstNode(Type::ANY) {}
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

  GroupNode(std::unique_ptr<AstNode>&& glob)
    : AstNode(Type::GROUP)
    , glob_{std::move(glob)} {}

  AstNode* GetGlob() {
    return glob_.get();
  }

 private:
  std::unique_ptr<AstNode> glob_;

};

class Glob {
 public:
  Glob() = default;
  void Parser(const std::string& pattern) {
    size_t pos = 0;
    int preview_state = -1;

    while(pos < pattern.length()) {
      size_t current_state = 0;
      char c = pattern[pos];
      switch (c) {
        case '?': {
          current_state = automato_.NewState<StateQuestion>();
          ++pos;
          break;
        }

        case '*': {
          current_state = automato_.NewState<StateMult>();
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

  Automata& GetAutomato() {
    return automato_;
  }
 private:
  Automata automato_;
};