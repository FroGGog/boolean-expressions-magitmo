#include "boolean_expressions/analyzer.h"

#include <stdexcept>
#include <utility>

//for debug
#include <iostream>

namespace boolean_expressions {

Tokenizer::Tokenizer(std::string_view input)
  : source_(input), pos_(0) { }

const std::vector<Token> &Tokenizer::ParseInput()
{
  tokens_.clear();

    while(true)
    {
      Token token = NextToken();
      tokens_.push_back(token);

      if(token.kType == TokenType::kEOI)
      {
        break;
      }
    }
    return GetTokens();
}

Token Tokenizer::NextToken()
{
    SkipWhiteSpace();
    if(!HasMore())
    {
      return {TokenType::kEOI, SourceSpan{pos_, pos_}, ""};
    }
    std::size_t begin = pos_;
    char current = source_[pos_];

    switch (current)
    {
    case '!':
      pos_++;
      return {TokenType::kNot, SourceSpan{begin, pos_}, source_.substr(begin, pos_ - begin)};
    case '&':
      pos_++;
      return {TokenType::kAnd, SourceSpan{begin, pos_}, source_.substr(begin, pos_ - begin)};
    case '|':
      pos_++;
      return {TokenType::kOr, SourceSpan{begin, pos_}, source_.substr(begin, pos_ - begin)};
    case '(':
      pos_++;
      return {TokenType::kLeftBracket, SourceSpan{begin, pos_}, source_.substr(begin, pos_ - begin)};
    case ')':
      pos_++;
      return {TokenType::kRightBracket, SourceSpan{begin, pos_}, source_.substr(begin, pos_ - begin)};
    default:
      break;
    }

    if(std::isalnum(static_cast<unsigned char>(current)) || current == '_')
    {
      return ScanIdentifier();
    }

    // for unknown tokens
    pos_++;
    return {TokenType::kUnknown, SourceSpan{begin, pos_}, source_.substr(begin, 1)};
}

bool Tokenizer::HasMore() const
{
    return pos_ < source_.size();
}

const std::vector<Token>& Tokenizer::GetTokens() const
{
  return tokens_;
}

void Tokenizer::SkipWhiteSpace()
{
  while(HasMore() && std::isspace(static_cast<unsigned char>(source_[pos_])))
  {
    pos_++;
  }
}

Token Tokenizer::ScanIdentifier()
{
  std::size_t begin = pos_;
  while (HasMore()) {
    char c = source_[pos_];
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') 
    {
      pos_++;
    } 
    else 
    {
      break;
    }
  }
  std::string_view identifier = source_.substr(begin, pos_ - begin);
  TokenType type = TokenType::kIdentifier;
  if(identifier == "true")
  {
    type = TokenType::kTrue;
  }
  else if (identifier == "false")
  {
    type = TokenType::kFalse;
  }
  return {type, SourceSpan{begin, pos_}, identifier};

}

struct ValidatedProgram::Impl {
  std::string debug_string;
};

ValidatedProgram::ValidatedProgram(std::unique_ptr<Impl> impl)
    : impl_(std::move(impl)) {}

ValidatedProgram::ValidatedProgram(ValidatedProgram&&) noexcept = default;
ValidatedProgram& ValidatedProgram::operator=(ValidatedProgram&&) noexcept =
    default;
ValidatedProgram::~ValidatedProgram() = default;

std::string ValidatedProgram::DebugString() const {
  return impl_->debug_string;
}

BuildResult BuildProgram(const ProgramSource& source) {
  static_cast<void>(source);
  throw std::logic_error("TODO: BuildProgram is not implemented");
}

}  // namespace boolean_expressions
