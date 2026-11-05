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

// Replace and extend these wiring tests with parameterized parser, AST and
// validation tests over every case from public_cases.h.

}  // namespace

