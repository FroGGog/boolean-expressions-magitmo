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

  EXPECT_EQ(parsed_string.size(), 3);

  EXPECT_EQ(parsed_string[0].text, "a");
  EXPECT_EQ(parsed_string[0].kType, boolean_expressions::TokenType::kIdentifier);

  EXPECT_EQ(parsed_string[1].text, "&");
  EXPECT_EQ(parsed_string[1].kType, boolean_expressions::TokenType::kAnd);

  EXPECT_EQ(parsed_string[2].text, "b");
  EXPECT_EQ(parsed_string[2].kType, boolean_expressions::TokenType::kIdentifier);
}


// Replace and extend these wiring tests with parameterized parser, AST and
// validation tests over every case from public_cases.h.

}  // namespace

