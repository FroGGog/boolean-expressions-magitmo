#include <iostream>
#include <stdexcept>
#include <variant>

#include "boolean_expressions/analyzer.h"

int main() {
  boolean_expressions::ProgramSource source{
      .inputs = {"has_token", "office_network", "malware_detected"},
      .expressions =
          {
              {"employee", "has_token | office_network"},
              {"trusted", "employee & !malware_detected"},
              {"allow", "trusted & has_token"},
          },
      .outputs = {"allow"},
  };

  try {
    auto result = boolean_expressions::BuildProgram(source);
    if (std::holds_alternative<boolean_expressions::ValidatedProgram>(result)) {
      const auto& program =
          std::get<boolean_expressions::ValidatedProgram>(result);
      std::cout << program.DebugString();
      return 0;
    }

    for (const auto& diagnostic :
         std::get<std::vector<boolean_expressions::Diagnostic>>(result)) {
      std::cerr << diagnostic.expression_name << ": " << diagnostic.message
                << '\n';
    }
    return 1;
  } catch (const std::logic_error& error) {
    std::cout << error.what() << '\n';
    return 0;
  }
}
