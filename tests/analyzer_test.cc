#include <gtest/gtest.h>

#include "boolean_expressions/analyzer.h"
#include "public_cases.h"

namespace {

TEST(Starter, ProgramSourceCanBeConstructed) {
  const boolean_expressions::ProgramSource source{
      .inputs = {"has_token"},
      .expressions = {{"allow", "has_token"}},
      .outputs = {"allow"},
  };

  ASSERT_EQ(source.expressions.size(), 1u);
  EXPECT_EQ(source.expressions.front().name, "allow");
}

TEST(Starter, PublicCasesAreAvailable) {
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


// Replace and extend these wiring tests with parameterized parser, AST and
// validation tests over every case from public_cases.h.

}  // namespace

