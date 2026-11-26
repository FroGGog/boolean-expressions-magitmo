#include <gtest/gtest.h>

#include "boolean_expressions/analyzer.h"
#include "public_cases.h"

namespace {

TEST(Starter, ProgramSourceCanBeConstructed) 
{
  const boolean_expressions::ProgramSource source{
      .inputs = {"has_token"},
      .expressions = {{"allow", "has_token"}},
      .outputs = {"allow"},
  };

  ASSERT_EQ(source.expressions.size(), 1u);
  EXPECT_EQ(source.expressions.front().name, "allow");
}

TEST(Starter, PublicCasesAreAvailable) 
{
  EXPECT_FALSE(boolean_expressions::test::PublicValidCases().empty());
  EXPECT_FALSE(boolean_expressions::test::PublicInvalidCases().empty());
}

TEST(Tokenizer, SimpleExpression)
{
  boolean_expressions::Tokenizer t("a & b");
  auto parsed_string = t.ParseInput();

  EXPECT_EQ(parsed_string.size(), 4);

  EXPECT_EQ(parsed_string[0].text, "a");
  EXPECT_EQ(parsed_string[0].kType, boolean_expressions::TokenType::kIdentifier);

  EXPECT_EQ(parsed_string[1].text, "&");
  EXPECT_EQ(parsed_string[1].kType, boolean_expressions::TokenType::kAnd);

  EXPECT_EQ(parsed_string[2].text, "b");
  EXPECT_EQ(parsed_string[2].kType, boolean_expressions::TokenType::kIdentifier);
}

TEST(Tokenizer, TokenTypeRecognition)
{
  boolean_expressions::Tokenizer t("!");
  t.ParseInput();
  boolean_expressions::Tokenizer t2("&");
  t2.ParseInput();
  boolean_expressions::Tokenizer t3("|");
  t3.ParseInput();
  boolean_expressions::Tokenizer t4("(");
  t4.ParseInput();
  boolean_expressions::Tokenizer t5(")");
  t5.ParseInput();
  boolean_expressions::Tokenizer t6("");
  t6.ParseInput();
  boolean_expressions::Tokenizer t7(";");
  t7.ParseInput();

  EXPECT_EQ(t.GetTokens()[0].kType, boolean_expressions::TokenType::kNot);
  EXPECT_EQ(t2.GetTokens()[0].kType, boolean_expressions::TokenType::kAnd);
  EXPECT_EQ(t3.GetTokens()[0].kType, boolean_expressions::TokenType::kOr);
  EXPECT_EQ(t4.GetTokens()[0].kType, boolean_expressions::TokenType::kLeftBracket);
  EXPECT_EQ(t5.GetTokens()[0].kType, boolean_expressions::TokenType::kRightBracket);
  EXPECT_EQ(t6.GetTokens()[0].kType, boolean_expressions::TokenType::kEOI);
  EXPECT_EQ(t7.GetTokens()[0].kType, boolean_expressions::TokenType::kUnknown);
}

TEST(Tokenizer, EmptyExpression)
{
  boolean_expressions::Tokenizer t("");
  auto parsed_string = t.ParseInput();

  EXPECT_EQ(parsed_string.size(), 1);
  EXPECT_EQ(parsed_string[0].kType, boolean_expressions::TokenType::kEOI);
}

TEST(Tokenizer, Whitespaces)
{
  boolean_expressions::Tokenizer tokenizer(" \t a \n & \r ! \t b ");
  auto tokens = tokenizer.ParseInput();

  EXPECT_EQ(tokens.size() , 5u);
  EXPECT_EQ(tokens[0].text, "a");
  EXPECT_EQ(tokens[1].kType, boolean_expressions::TokenType::kAnd);
  EXPECT_EQ(tokens[2].kType, boolean_expressions::TokenType::kNot);
  EXPECT_EQ(tokens[3].text, "b");
}

template<typename T>
const T* AsNode(const boolean_expressions::AstNode* node)
{
  const T* casted = dynamic_cast<const T*>(node);
  EXPECT_NE(casted, nullptr) << "Node type mismatch";
  return casted;
}

TEST(Parser, SingleIdentifier) 
{
  boolean_expressions::Tokenizer tokenizer("a");
  boolean_expressions::Parser parser("test_expr", tokenizer.ParseInput());
    
  auto ast = parser.Parse();
  ASSERT_TRUE(ast != nullptr);
  const auto* literal = AsNode<boolean_expressions::LiteralNode>(ast.get());
    
  EXPECT_EQ(literal->value, "a");
  EXPECT_EQ(literal->type, boolean_expressions::TokenType::kIdentifier);
  EXPECT_EQ(literal->span.begin, 0);
  EXPECT_EQ(literal->span.end, 1);
}

TEST(Parser, OperatorPrecedence_AndBeforeOr) 
{
  boolean_expressions::Tokenizer tokenizer("a & b | c");
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
  auto ast = parser.Parse();

  const auto* or_node = AsNode<boolean_expressions::BinaryOperatorNode>(ast.get());
  EXPECT_EQ(or_node->op, boolean_expressions::TokenType::kOr);

  const auto* and_node = AsNode<boolean_expressions::BinaryOperatorNode>(or_node->left.get());
  EXPECT_EQ(and_node->op, boolean_expressions::TokenType::kAnd);
    
  const auto* ref_a = AsNode<boolean_expressions::LiteralNode>(and_node->left.get());
  EXPECT_EQ(ref_a->value, "a");
    
  const auto* ref_b = AsNode<boolean_expressions::LiteralNode>(and_node->right.get());
  EXPECT_EQ(ref_b->value, "b");

  const auto* ref_c = AsNode<boolean_expressions::LiteralNode>(or_node->right.get());
  EXPECT_EQ(ref_c->value, "c");
}

TEST(Parser, SyntaxError_TrailingInput)
{
  boolean_expressions::Tokenizer tokenizer("a b"); 
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
    
  EXPECT_THROW(parser.Parse(), boolean_expressions::ParseError);
}

TEST(Parser, SyntaxError_MissingOperand) 
{
  boolean_expressions::Tokenizer tokenizer("a &");
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
    
  EXPECT_THROW(parser.Parse(), boolean_expressions::ParseError);
}

TEST(Parser, SyntaxError_UnclosedParenthesis) 
{
  boolean_expressions::Tokenizer tokenizer("(a & b");
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
    
  EXPECT_THROW(parser.Parse(), boolean_expressions::ParseError);
}

TEST(Parser, DeeplyNestedParentheses)
{
  boolean_expressions::Tokenizer tokenizer("(((a)))");
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
  auto ast = parser.Parse();

  const auto& ref = AsNode<boolean_expressions::LiteralNode>(ast.get());
  EXPECT_EQ(ref->value, "a");
}

TEST(Parser, UnaryOperators_Chained)
{
  boolean_expressions::Tokenizer tokenizer("!!a");
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
  auto ast = parser.Parse();

  const auto* root1 = AsNode<boolean_expressions::UnaryOperatorNode>(ast.get());
  EXPECT_EQ(root1->op, boolean_expressions::TokenType::kNot);

  const auto* root2 = AsNode<boolean_expressions::UnaryOperatorNode>(root1->operand.get());
  EXPECT_EQ(root2->op, boolean_expressions::TokenType::kNot);

  const auto* root3 = AsNode<boolean_expressions::LiteralNode>(root2->operand.get());
  EXPECT_EQ(root3->type, boolean_expressions::TokenType::kIdentifier);
}

TEST(Parser, ParanthesesOverridePrecedence)
{
  boolean_expressions::Tokenizer tokenizer("(a | b) & c");
  boolean_expressions::Parser parser("test", tokenizer.ParseInput());
  auto ast = parser.Parse();

  const auto* and_node = AsNode<boolean_expressions::BinaryOperatorNode>(ast.get());
  EXPECT_EQ(and_node->op, boolean_expressions::TokenType::kAnd);

  const auto* or_node = AsNode<boolean_expressions::BinaryOperatorNode>(and_node->left.get());
  EXPECT_EQ(or_node->op, boolean_expressions::TokenType::kOr);

  const auto* ref_a = AsNode<boolean_expressions::LiteralNode>(or_node->left.get());
  EXPECT_EQ(ref_a->value, "a");
    
  const auto* ref_b = AsNode<boolean_expressions::LiteralNode>(or_node->right.get());
  EXPECT_EQ(ref_b->value, "b");

  const auto* ref_c = AsNode<boolean_expressions::LiteralNode>(and_node->right.get());
  EXPECT_EQ(ref_c->value, "c");  

}

// Replace and extend these wiring tests with parameterized parser, AST and
// validation tests over every case from public_cases.h.

}  // namespace

