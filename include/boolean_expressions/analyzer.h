#ifndef BOOLEAN_EXPRESSIONS_ANALYZER_H_
#define BOOLEAN_EXPRESSIONS_ANALYZER_H_

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <cctype>
#include <fstream>

namespace boolean_expressions {

enum class TokenType
{
  kIdentifier,
  kTrue,
  kFalse,
  kNot,
  kAnd,
  kOr,
  kRightBracket,
  kLeftBracket,
  kEOI,
  kUnknown
};

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

struct Token
{
  TokenType kType;
  SourceSpan span;
  std::string_view text;
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

class Tokenizer
{
public:

  explicit Tokenizer(std::string_view input);

  const std::vector<Token>& ParseInput();

  const std::vector<Token>& GetTokens() const;

private:
  std::string_view source_;
  std::size_t pos_;

  std::vector<Token> tokens_;

  Token NextToken();

  bool HasMore() const;

  void SkipWhiteSpace();
  // return identifer that is not simple token as (&, |, )
  Token ScanIdentifier();
  
};

class AstNode
{
public:
  virtual ~AstNode() = default;
  const SourceSpan span;
protected:
  explicit AstNode(SourceSpan s) : span(s) {}
};

class LiteralNode : public AstNode
{
public:
  const std::string_view value;
  const TokenType type;

  LiteralNode(SourceSpan s, std::string_view v, TokenType t)
    : AstNode(s), value(v), type(t)
  {}
};

class UnaryOperatorNode : public AstNode
{
public:
  const TokenType op;
  const std::unique_ptr<const AstNode> operand;

  UnaryOperatorNode(SourceSpan s, TokenType o, std::unique_ptr<const AstNode> opnd)
    : AstNode(s), op(o), operand(std::move(opnd)) {}
};

class BinaryOperatorNode : public AstNode
{
public:
  using Node = std::unique_ptr<const AstNode>;
  const TokenType op;
  const Node left;
  const Node right;

  BinaryOperatorNode(SourceSpan s, TokenType o, Node l, Node r)
    : AstNode(s), op(o), left(std::move(l)), right(std::move(r)) {}
};

class ParseError : public std::runtime_error
{
public:
  explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class Parser
{
public:
  Parser(std::string expr_name, const std::vector<Token>& tokens)
        : expr_name_(std::move(expr_name)), tokens_(tokens), current_pos_(0) {}

  using Node = std::unique_ptr<const AstNode>;
  Node Parse();

private:
std::string expr_name_;
  std::vector<Token> tokens_;
  std::size_t current_pos_ = 0;

  const Token& CurrentToken() const;
  void Advance();
  void Expect(TokenType expected);

  Node ParseExpression();
  Node ParseTerm();
  Node ParseFactor();

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
