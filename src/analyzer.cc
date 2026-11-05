#include "boolean_expressions/analyzer.h"

#include <stdexcept>
#include <utility>

namespace boolean_expressions {

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
