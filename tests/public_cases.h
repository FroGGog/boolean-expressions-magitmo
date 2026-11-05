#ifndef BOOLEAN_EXPRESSIONS_TESTS_PUBLIC_CASES_H_
#define BOOLEAN_EXPRESSIONS_TESTS_PUBLIC_CASES_H_

#include <string>
#include <vector>

#include "boolean_expressions/analyzer.h"

namespace boolean_expressions::test {

struct ValidCase {
  std::string name;
  ProgramSource source;
  std::string expected_debug_string;
};

struct InvalidCase {
  std::string name;
  ProgramSource source;
  DiagnosticCode expected_code;
  std::vector<std::string> expected_dependency_path;
};

inline std::vector<ValidCase> PublicValidCases() {
  return {
      {
          .name = "operator precedence",
          .source =
              {
                  .inputs = {"a", "b", "c"},
                  .expressions = {{"result", "a | b & c"}},
                  .outputs = {"result"},
              },
          .expected_debug_string =
              "result = (OR (REF a) (AND (REF b) (REF c)))\n",
      },
      {
          .name = "parentheses override precedence",
          .source =
              {
                  .inputs = {"a", "b", "c"},
                  .expressions = {{"result", "(a | b) & c"}},
                  .outputs = {"result"},
              },
          .expected_debug_string =
              "result = (AND (OR (REF a) (REF b)) (REF c))\n",
      },
      {
          .name = "unary operators",
          .source =
              {
                  .inputs = {"a", "b", "c"},
                  .expressions = {{"result", "!a | !b & !c"}},
                  .outputs = {"result"},
              },
          .expected_debug_string =
              "result = (OR (NOT (REF a)) "
              "(AND (NOT (REF b)) (NOT (REF c))))\n",
      },
      {
          .name = "boolean literals",
          .source =
              {
                  .inputs = {},
                  .expressions = {{"result", "true & !false"}},
                  .outputs = {"result"},
              },
          .expected_debug_string =
              "result = (AND (LITERAL true) (NOT (LITERAL false)))\n",
      },
      {
          .name = "named expression dependencies",
          .source =
              {
                  .inputs = {"has_token", "office_network"},
                  .expressions =
                      {
                          {"employee", "has_token | office_network"},
                          {"allow", "employee & has_token"},
                      },
                  .outputs = {"allow"},
              },
          .expected_debug_string =
              "employee = (OR (REF has_token) (REF office_network))\n"
              "allow = (AND (REF employee) (REF has_token))\n",
      },
  };
}

inline std::vector<InvalidCase> PublicInvalidCases() {
  return {
      {
          .name = "unknown reference",
          .source =
              {
                  .inputs = {"a"},
                  .expressions = {{"result", "a & missing"}},
                  .outputs = {"result"},
              },
          .expected_code = DiagnosticCode::kUnknownReference,
          .expected_dependency_path = {},
      },
      {
          .name = "unknown output",
          .source =
              {
                  .inputs = {"a"},
                  .expressions = {{"result", "a"}},
                  .outputs = {"missing"},
              },
          .expected_code = DiagnosticCode::kUnknownOutput,
          .expected_dependency_path = {},
      },
      {
          .name = "duplicate expression name",
          .source =
              {
                  .inputs = {"a"},
                  .expressions = {{"result", "a"}, {"result", "!a"}},
                  .outputs = {"result"},
              },
          .expected_code = DiagnosticCode::kDuplicateName,
          .expected_dependency_path = {},
      },
      {
          .name = "input expression collision",
          .source =
              {
                  .inputs = {"result"},
                  .expressions = {{"result", "true"}},
                  .outputs = {"result"},
              },
          .expected_code = DiagnosticCode::kNameCollision,
          .expected_dependency_path = {},
      },
      {
          .name = "self cycle",
          .source =
              {
                  .inputs = {},
                  .expressions = {{"a", "!a"}},
                  .outputs = {"a"},
              },
          .expected_code = DiagnosticCode::kDependencyCycle,
          .expected_dependency_path = {"a", "a"},
      },
      {
          .name = "multi node cycle",
          .source =
              {
                  .inputs = {"input"},
                  .expressions =
                      {
                          {"a", "b & input"},
                          {"b", "!c"},
                          {"c", "a | input"},
                      },
                  .outputs = {"a"},
              },
          .expected_code = DiagnosticCode::kDependencyCycle,
          .expected_dependency_path = {"a", "b", "c", "a"},
      },
      {
          .name = "syntax error",
          .source =
              {
                  .inputs = {"a", "b"},
                  .expressions = {{"result", "a & | b"}},
                  .outputs = {"result"},
              },
          .expected_code = DiagnosticCode::kSyntaxError,
          .expected_dependency_path = {},
      },
  };
}

}  // namespace boolean_expressions::test

#endif  // BOOLEAN_EXPRESSIONS_TESTS_PUBLIC_CASES_H_

