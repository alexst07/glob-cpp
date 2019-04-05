#include <string>
#include <tuple>
#include <vector>
#include <memory>

class Automata;

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