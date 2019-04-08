#ifndef GLOB_CPP_TRAVERSAL_H
#define GLOB_CPP_TRAVERSAL_H

#include<iostream>
#include "glob.h"

class PrintTraversal: public AstVisitor {
 public:
  void Visit(AstNode *node) {
    level_ = 0;
    after_simple_ = false;
    node->Accept(this);
  }

  void VisitCharNode(CharNode* char_node) override {
    char c = char_node->GetValue();
    std::cout << "[" << c << "]";
    after_simple_ = true;
  }

  void VisitRangeNode(RangeNode* node) override {
    after_simple_ = false;
    NewLine(false);
    Level();
    std::cout << "<range start: ";
    node->GetStart()->Accept(this);
    std::cout << ", end: ";
    node->GetEnd()->Accept(this);
    std::cout << ">";
    NewLine();
    after_simple_ = false;
  }

  void VisitSetItemsNode(SetItemsNode* node) override {
    after_simple_ = false;
    auto& vec = node->GetItems();
    for (auto& item_node : vec) {
      item_node.get()->Accept(this);
    }
    after_simple_ = false;
  }

  void VisitPositiveSetNode(PositiveSetNode* node) override {
    NewLine(false);
    after_simple_ = false;
    Level();
    std::cout << "<set-positive>";
    NewLine();
    ++level_;
    node->GetSet()->Accept(this);
    --level_;
    Level();
    std::cout << "</set-positive>";
    NewLine();
    after_simple_ = false;
  }

  void VisitNegativeSetNode(NegativeSetNode* node) override {
    NewLine(false);
    after_simple_ = false;
    Level();
    std::cout << "<neg-positive>";
    NewLine();
    ++level_;
    node->GetSet()->Accept(this);
    --level_;
    Level();
    std::cout << "</neg-positive>";
    NewLine();

    after_simple_ = false;
  }

  void VisitStarNode(StarNode* node) override {
    std::cout << "[star]";
    after_simple_ = true;
  }

  void VisitAnyNode(AnyNode* node) override {
    std::cout << "[any]";
    after_simple_ = true;
  }

  void VisitGroupNode(GroupNode* node) override {
    NewLine(false);
    after_simple_ = false;
    Level();
    std::cout << "<group>";
    NewLine();
    ++level_;
    node->GetGlob()->Accept(this);
    --level_;
    Level();
    std::cout << "</group>";
    NewLine();
  }

  void VisitConcatNode(ConcatNode* node) override {
    NewLine( );
    after_simple_ = false;
    auto& vec = node->GetBasicGlobs();
    Level();
    std::cout << "<concat>";
    NewLine();
    ++level_;
    Level();
    for (auto& item_node : vec) {
      item_node.get()->Accept(this);
    }
    --level_;
    NewLine(false);
    Level();
    std::cout << "</concat>";
    NewLine();
    after_simple_ = false;
  }

  void VisitUnionNode(UnionNode* node) override {
    after_simple_ = false;
    auto& vec = node->GetItems();
    Level();
    std::cout << "<union>";
    NewLine();

    for (auto& item_node : vec) {
      ++level_;
      Level();
      std::cout << "<item>";
      item_node.get()->Accept(this);
      Level();
      std::cout << "</item>";
      --level_;
    }

    NewLine(false);
    Level();
    std::cout << "</union>";
    NewLine();
    after_simple_ = false;
  }

  void VisitGlobNode(GlobNode* node) override {
    after_simple_ = false;
    ++level_;
    Level();
    std::cout << "<glob>";
    NewLine();

    node->GetConcat()->Accept(this);

    Level();
    std::cout << "</glob>";
    NewLine();
    --level_;
    after_simple_ = false;
  }

 private:
  void NewLine(bool v = true) {
    if (v) {
      std::cout << "\n";
    } else {
      if (after_simple_) {
        std::cout << "\n";
      }
    }
  }
  void Level() {
    for (int i = 0; i < level_-1; i++) {
      std::cout << " │";
    }

    std::cout << " ├";
    std::cout << "─";
  }

  bool after_simple_;
  int level_;
};

void PrintAst(const std::string& str) {
  Lexer l(str);
  std::vector<Token> tokens = l.Scanner();
  Parser p(std::move(tokens));
  AstNodePtr ast_ptr = p.GenAst();
  PrintTraversal visitor;
  visitor.Visit(ast_ptr.get());
  std::cout << "\n";
}

#endif  // GLOB_CPP_TRAVERSAL_H