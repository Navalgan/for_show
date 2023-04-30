#pragma once

#include "parser.h"
#include <string>

std::vector<std::shared_ptr<Object>> Convert(Scope& scope, const std::shared_ptr<Object>& ast);

std::vector<std::shared_ptr<Object>> ConvertIf(Scope& scope, const std::shared_ptr<Object>& ast);

std::shared_ptr<Object> Solve(Scope& scope, const std::shared_ptr<Object>& ast);

class Interpreter {
public:
    std::string Run(const std::string& str);

    Scope global_scope_;
};
