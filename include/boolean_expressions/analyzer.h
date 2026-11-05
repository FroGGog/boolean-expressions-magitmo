#ifndef BOOLEAN_EXPRESSIONS_ANALYZER_H_
#define BOOLEAN_EXPRESSIONS_ANALYZER_H_

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace boolean_expressions {

struct ExpressionDefinition {
  std::string name;
  std::string text;
};

struct ProgramSource {
  std::vector<std::string> inputs;
  std::vector<ExpressionDefinition> expressions;
  std::vector<std::string> outputs;
};

struct SourceSpan {
  std::size_t begin = 0;
  std::size_t end = 0;
};

enum class DiagnosticCode {
  kSyntaxError,
  kDuplicateName,
  kNameCollision,
  kUnknownReference,
  kUnknownOutput,
  kDependencyCycle,
};

struct Diagnostic {
  DiagnosticCode code;
  std::string expression_name;
  std::string message;
  std::optional<SourceSpan> span;
  std::vector<std::string> dependency_path;
};

// Successful result of parsing and validating ProgramSource. Its internal AST
// representation is intentionally not specified by the public API.
class ValidatedProgram {
 public:
  ValidatedProgram(ValidatedProgram&&) noexcept;
  ValidatedProgram& operator=(ValidatedProgram&&) noexcept;
  ~ValidatedProgram();

  ValidatedProgram(const ValidatedProgram&) = delete;
  ValidatedProgram& operator=(const ValidatedProgram&) = delete;

  // Returns one expression per line in deterministic prefix form. This method
  // is intended for examples and black-box structural tests, not evaluation.
  std::string DebugString() const;

 private:
  struct Impl;

  explicit ValidatedProgram(std::unique_ptr<Impl> impl);

  std::unique_ptr<Impl> impl_;

  friend std::variant<ValidatedProgram, std::vector<Diagnostic>> BuildProgram(
      const ProgramSource& source);
};

using BuildResult =
    std::variant<ValidatedProgram, std::vector<Diagnostic>>;

// Parses all expressions, builds an immutable AST and validates names and
// dependencies. Throws std::logic_error in the initial stub only. The completed
// implementation must always return BuildResult instead.
BuildResult BuildProgram(const ProgramSource& source);

}  // namespace boolean_expressions

#endif  // BOOLEAN_EXPRESSIONS_ANALYZER_H_
