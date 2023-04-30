#include "object.h"
#include "function.h"
#include "scheme.h"

std::string PrintRec(const std::shared_ptr<Object>& obj) {
    std::string res;

    auto cell = As<Cell>(obj);

    if (cell->GetFirst()) {
        res += cell->GetFirst()->Serialize();
    }

    if (cell->GetSecond()) {
        if (Is<Cell>(cell->GetSecond())) {
            res += ' ';
            if (Is<Cell>(cell->GetSecond())) {
                res += PrintRec(cell->GetSecond());
            } else {
                res += cell->GetSecond()->Serialize();
            }
        } else {
            res += " . " + cell->GetSecond()->Serialize();
        }
    }

    return res;
}

Number::Number(int value) : value_(value) {
}

Number::Number(const std::string& value) : value_(atoi(value.c_str())) {
}

std::shared_ptr<Object> Number::Eval() {
    return std::shared_ptr<Object>(new Number(value_));
}

std::string Number::Serialize() {
    return std::to_string(value_);
}

int64_t Number::GetValue() const {
    return value_;
}

Symbol::Symbol(std::string_view str) : name_(str) {
}

std::shared_ptr<Object> Symbol::Eval() {
    if (name_ == "number?") {
        return std::shared_ptr<Object>(new Function(name_));
    }
    return std::shared_ptr<Object>(new Symbol(name_));
}

std::string Symbol::Serialize() {
    return name_;
}

const std::string& Symbol::GetName() const {
    return name_;
}

Cell::Cell() : first_(nullptr), second_(nullptr) {
}

Cell::Cell(const std::shared_ptr<Object>& first) : first_(first), second_(nullptr) {
}

std::shared_ptr<Object> Cell::Eval() {
    return std::shared_ptr<Object>(new Cell());
}

std::string Cell::Serialize() {
    std::string res(1, '(');

    if (!first_) {
        return "(())";
    }

    if (Is<Cell>(second_)) {
        auto for_test = As<Cell>(second_)->GetFirst();
        if (Is<Symbol>(for_test)) {
            auto for_quote = As<Symbol>(for_test)->GetName();
            if (for_quote == "\'" || for_quote == "quote") {
                return first_->Serialize();
            }
        }
    }

    auto cell = std::shared_ptr<Cell>(new Cell(first_));
    cell->SetSecond(second_);

    res += PrintRec(cell);

    res += ')';

    return res;
}

void Cell::SetFirst(const std::shared_ptr<Object>& first) {
    first_ = first;
}

void Cell::SetSecond(const std::shared_ptr<Object>& second) {
    second_ = second;
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return first_;
}
std::shared_ptr<Object> Cell::GetSecond() const {
    return second_;
}
std::shared_ptr<Object> Cell::GetSecond() {
    return second_;
}

std::shared_ptr<Object> Scope::GetValue(const std::string& str) {
    if (scope_.find(str) == scope_.end()) {
        return nullptr;
    }
    return scope_[str];
}

std::unordered_map<std::string, std::shared_ptr<Object>> Scope::GetScope() {
    return scope_;
}

bool Scope::SetValue(const std::string& str, std::shared_ptr<Object> obj) {
    scope_[str] = obj;
    return true;
}

bool Scope::Set(const std::string& str, std::shared_ptr<Object> obj) {
    if (scope_.find(str) == scope_.end()) {
        throw NameError("No name");
    }
    scope_[str] = obj;
    return true;
}

Lambda::Lambda() {
}

Lambda::Lambda(bool flag) {
    plug = flag;
}

Lambda::Lambda(Scope& scope, const std::vector<std::shared_ptr<Object>>& var,
               const std::vector<std::shared_ptr<Object>>& exp)
    : variables_(var), expressions_(exp), scope_(scope) {
}

std::shared_ptr<Object> Lambda::Eval() {
    return nullptr;
}

std::string Lambda::Serialize() {
    return "";
}

std::shared_ptr<Object> Lambda::Run(Scope& scope,
                                    const std::vector<std::shared_ptr<Object>>& args) {
    if (!variables_.empty() && args.size() != variables_.size()) {
        throw SyntaxError("");
    }

    for (const auto& [key, value] : scope.GetScope()) {
        if (scope_.GetScope().find(key) == scope_.GetScope().end()) {
            scope_.SetValue(key, value);
        }
    }

    for (size_t i = 0; i < variables_.size(); ++i) {
        auto var = As<Symbol>(variables_[i])->GetName();
        scope_.SetValue(var, args[i]);
    }

    std::shared_ptr<Object> res;

    for (const auto& exp : expressions_) {
        if (Is<Cell>(exp)) {
            if (Is<Symbol>(As<Cell>(exp)->GetFirst())) {
                if (As<Symbol>(As<Cell>(exp)->GetFirst())->GetName() == "lambda") {
                    auto lambda_exp = ConvertIf(scope_, As<Cell>(exp)->GetSecond());

                    Function lambda_func;

                    res = lambda_func.LambdaFunc(scope_, {}, lambda_exp, {});
                    continue;
                }
            }
        }
        res = Solve(scope_, exp);
    }

    return res;
}

Scope& Lambda::GetScope() {
    return scope_;
}

void Lambda::SetScope(Scope& scope) {
    scope_ = scope;
}
