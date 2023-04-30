#include "function.h"
#include "parser.h"
#include "scheme.h"
#include <memory>

Function::Function() {
}

Function::Function(std::string_view str) : name_of_func_(str) {
}

std::shared_ptr<Object> Function::Apply(Scope& scope,
                                        const std::vector<std::shared_ptr<Object>>& args) {
    std::string answer;
    if (name_of_func_ == "number?") {
        answer = IsNumber(args[0]) ? "#t" : "#f";
    } else if (name_of_func_ == "=") {
        answer = Comparisons(std::equal_to(), args) ? "#t" : "#f";
    } else if (name_of_func_ == ">") {
        answer = Comparisons(std::greater(), args) ? "#t" : "#f";
    } else if (name_of_func_ == "<") {
        answer = Comparisons(std::less(), args) ? "#t" : "#f";
    } else if (name_of_func_ == ">=") {
        answer = Comparisons(std::greater_equal(), args) ? "#t" : "#f";
    } else if (name_of_func_ == "<=") {
        answer = Comparisons(std::less_equal(), args) ? "#t" : "#f";
    } else if (name_of_func_ == "+") {
        if (args.empty()) {
            return std::shared_ptr<Object>(new Number(0));
        }
        return Arithmetic(std::plus(), args);
    } else if (name_of_func_ == "-") {
        if (args.empty()) {
            throw RuntimeError("No args");
        }
        return Arithmetic(std::minus(), args);
    } else if (name_of_func_ == "*") {
        if (args.empty()) {
            return std::shared_ptr<Object>(new Number(1));
        }
        return Arithmetic(std::multiplies(), args);
    } else if (name_of_func_ == "/") {
        if (args.empty()) {
            throw RuntimeError("No args");
        }
        return Arithmetic(std::divides(), args);
    } else if (name_of_func_ == "max" || name_of_func_ == "min") {
        return DoubleM(name_of_func_, args);
    } else if (name_of_func_ == "abs") {
        if (args.size() != 1) {
            throw RuntimeError("Need only one arg");
        }
        return Abs(args[0]);
    } else if (name_of_func_ == "boolean?") {
        answer = IsBoolean(args[0]) ? "#t" : "#f";
    } else if (name_of_func_ == "not") {
        answer = Not(args) ? "#t" : "#f";
    } else if (name_of_func_ == "and") {
        And(answer, args);
    } else if (name_of_func_ == "or") {
        Or(answer, args);
    } else if (name_of_func_ == "pair?") {
        answer = IsPair(args[0]) ? "#t" : "#f";
    } else if (name_of_func_ == "null?") {
        answer = IsNull(args[0]) ? "#t" : "#f";
    } else if (name_of_func_ == "list?") {
        answer = IsList(args[0]) ? "#t" : "#f";
    } else if (name_of_func_ == "cons") {
        return Cons(scope, args);
    } else if (name_of_func_ == "car") {
        auto res = Car(scope, args[0]);
        if (res) {
            return res;
        }
        answer = "()";
    } else if (name_of_func_ == "cdr") {
        auto res = Cdr(scope, args[0]);
        if (res) {
            return res;
        }
        answer = "()";
    } else if (name_of_func_ == "list") {
        List(answer, args);
    } else if (name_of_func_ == "list-ref") {
        ListRef(answer, args);
    } else if (name_of_func_ == "list-tail") {
        ListTail(answer, args);
    } else if (name_of_func_ == "symbol?") {
        answer = IsSymbol(args[0]) ? "#t" : "#f";
    } else if (name_of_func_ == "define") {
        answer = Define(scope, args) ? "#t" : "#f";
    } else if (name_of_func_ == "set!") {
        answer = Set(scope, args) ? "#t" : "#f";
    } else if (name_of_func_ == "if") {
        return If(scope, args);
    } else if (name_of_func_ == "set-car!") {
        answer = SetCar(scope, args) ? "#t" : "#f";
    } else if (name_of_func_ == "set-cdr!") {
        answer = SetCdr(scope, args) ? "#t" : "#f";
    }
    if (Is<Lambda>(scope.GetValue(name_of_func_))) {
        auto lambda = As<Lambda>(scope.GetValue(name_of_func_));
        return lambda->Run(scope, args);
    }
    return std::shared_ptr<Object>(new Symbol(answer));
}

std::shared_ptr<Object> Function::LambdaFunc(Scope& scope,
                                             const std::vector<std::shared_ptr<Object>>& var,
                                             const std::vector<std::shared_ptr<Object>>& exp,
                                             const std::vector<std::shared_ptr<Object>>& args) {

    std::shared_ptr<Lambda> lambda(new Lambda(scope, var, exp));

    if (args.empty()) {
        return lambda;
    }

    return lambda->Run(scope, args);
}

std::string Function::GetName() {
    return name_of_func_;
}

std::shared_ptr<Object> Function::Eval() {
    return nullptr;
}

std::string Function::Serialize() {
    return std::string();
}

bool Function::IsNumber(const std::shared_ptr<Object>& obj) {
    return Is<Number>(obj);
}

std::shared_ptr<Object> Function::DoubleM(std::string_view str,
                                          const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.empty()) {
        throw RuntimeError("No args");
    }
    if (!Is<Number>(objs[0])) {
        throw RuntimeError("No number");
    }
    int64_t res = As<Number>(objs[0])->GetValue();

    if (str == "min") {
        for (size_t i = 1; i < objs.size(); ++i) {
            if (Is<Number>(objs[i])) {
                res = std::min(res, As<Number>(objs[i])->GetValue());
            } else {
                throw RuntimeError("No number");
            }
        }
    } else {
        for (size_t i = 1; i < objs.size(); ++i) {
            res = std::max(res, As<Number>(objs[i])->GetValue());
        }
    }

    return std::shared_ptr<Object>(new Number(res));
}

std::shared_ptr<Object> Function::Abs(const std::shared_ptr<Object>& obj) {
    if (!Is<Number>(obj)) {
        throw RuntimeError("No number");
    }
    return std::shared_ptr<Object>(new Number(std::abs(As<Number>(obj)->GetValue())));
}

bool Function::IsBoolean(const std::shared_ptr<Object>& obj) {
    if (Is<Symbol>(obj)) {
        std::string str_bool = As<Symbol>(obj)->GetName();
        if (str_bool == "#t" || str_bool == "#f") {
            return true;
        }
        return false;
    }
    return false;
}

bool Function::Not(const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.size() != 1) {
        throw RuntimeError("No objs");
    }
    if (Is<Symbol>(objs[0])) {
        std::string str_bool = As<Symbol>(objs[0])->GetName();
        if (str_bool == "#t") {
            return false;
        } else if (str_bool == "#f") {
            return true;
        }
    }
    return false;
}

void Function::And(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs) {
    answer = "#t";
    if (objs.empty()) {
        return;
    }
    for (const auto& obj : objs) {
        if (Is<Symbol>(obj)) {
            auto text = As<Symbol>(obj)->GetName();
            if (text == "#t") {
                continue;
            } else if (text == "#f") {
                answer = "#f";
                return;
            }
        }
    }
    answer = objs.back()->Serialize();
}

void Function::Or(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs) {
    answer = "#f";
    if (objs.empty()) {
        return;
    }
    for (const auto& obj : objs) {
        if (Is<Symbol>(obj)) {
            auto text = As<Symbol>(obj)->GetName();
            if (text == "#t") {
                answer = "#t";
                return;
            } else if (text == "#f") {
                answer = "#f";
            }
        }
    }
    answer = objs.back()->Serialize();
}

bool Function::IsPair(const std::shared_ptr<Object>& obj) {
    std::stringstream ss{As<Symbol>(obj)->GetName()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    if (!input_ast) {
        return false;
    }

    auto cell = As<Cell>(input_ast);

    if (!Is<Cell>(cell->GetSecond())) {
        return true;
    }

    cell = As<Cell>(cell->GetSecond());

    if (!cell->GetSecond()) {
        return true;
    }

    return false;
}

bool Function::IsNull(const std::shared_ptr<Object>& obj) {
    std::stringstream ss{As<Symbol>(obj)->GetName()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    if (!input_ast) {
        return true;
    }

    return false;
}

bool Function::IsList(const std::shared_ptr<Object>& obj) {
    std::stringstream ss{As<Symbol>(obj)->GetName()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    if (!input_ast) {
        return true;
    }

    auto cell = As<Cell>(input_ast);

    while (cell->GetSecond()) {
        if (!Is<Cell>(cell->GetSecond())) {
            return false;
        }
        cell = As<Cell>(cell->GetSecond());
    }

    return true;
}

std::shared_ptr<Object> Function::Cons(Scope& scope,
                                       const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.size() != 2) {
        throw RuntimeError("Need only 2 args");
    }
    std::shared_ptr<Cell> cell(new Cell(Solve(scope, objs[0])));
    cell->SetSecond(Solve(scope, objs[1]));

    return cell;
}

std::shared_ptr<Object> Function::Car(Scope& scope, const std::shared_ptr<Object>& obj) {
    if (Is<Cell>(obj)) {
        auto lambda_check = As<Cell>(obj);
        if (Is<Lambda>(lambda_check->GetFirst())) {
            auto lambda = As<Lambda>(lambda_check->GetFirst());

            auto res = lambda->Run(scope, {});

            As<Lambda>(lambda_check->GetSecond())->SetScope(lambda->GetScope());

            return res;
        }
    }
    std::stringstream ss{As<Symbol>(obj)->GetName()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    if (!input_ast) {
        throw RuntimeError("Need first arg");
    }

    if (!Is<Cell>(input_ast)) {
        return Car(scope, Solve(scope, obj));
    }

    return As<Cell>(input_ast)->GetFirst();
}

std::shared_ptr<Object> Function::Cdr(Scope& scope, const std::shared_ptr<Object>& obj) {
    if (Is<Cell>(obj)) {
        auto lambda_check = As<Cell>(obj);
        if (Is<Lambda>(lambda_check->GetSecond())) {
            auto lambda = As<Lambda>(lambda_check->GetSecond());

            auto res = lambda->Run(scope, {});

            As<Lambda>(lambda_check->GetFirst())->SetScope(lambda->GetScope());

            return res;
        }
    }
    std::stringstream ss{As<Symbol>(obj)->GetName()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    if (!input_ast) {
        throw RuntimeError("Need second arg");
    }

    if (!Is<Cell>(input_ast)) {
        return Cdr(scope, Solve(scope, obj));
    }

    return As<Cell>(input_ast)->GetSecond();
}

void Function::List(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.empty()) {
        answer = "()";
        return;
    }

    std::shared_ptr<Cell> cell_root(new Cell(objs[0]));

    auto ans = cell_root;

    for (size_t i = 1; i < objs.size(); ++i) {
        auto second_cell = std::make_shared<Cell>(objs[i]);
        cell_root->SetSecond(second_cell);
        cell_root = second_cell;
    }

    answer = ans->Serialize();
}

void Function::ListRef(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs) {
    auto real_args = As<Symbol>(objs[0])->GetName();
    std::stringstream ss{real_args.substr(1, real_args.size() - 3)};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    Scope plug;

    auto args = Convert(plug, input_ast);

    auto pos = atoi(real_args.substr(args.size() + 2 + args.size(), real_args.size() - 1).c_str());

    if (pos > args.size() - 1) {
        throw RuntimeError("pos is doesn't cool");
    }

    answer = args[pos]->Serialize();
}

void Function::ListTail(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs) {
    auto real_args = As<Symbol>(objs[0])->GetName();
    std::stringstream ss{real_args.substr(1, real_args.size() - 3)};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    auto cell = As<Cell>(input_ast);

    Scope plug;

    auto args = Convert(plug, input_ast);

    auto pos = atoi(real_args.substr(args.size() + 2 + args.size(), real_args.size() - 1).c_str());

    while (pos) {
        --pos;
        if (cell->GetSecond()) {
            cell = As<Cell>(cell->GetSecond());
        } else {
            if (pos) {
                throw RuntimeError("pos is doesn't cool");
            }
            answer = "()";
            return;
        }
    }

    answer = cell->Serialize();
}

bool Function::IsSymbol(const std::shared_ptr<Object>& obj) {
    return Is<Symbol>(obj);
}

bool Function::Define(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.empty() || objs.size() != 2) {
        throw SyntaxError("No args");
    }
    if (Is<Object>(objs[0])) {
        return scope.SetValue(As<Symbol>(objs[0])->GetName(), objs[1]);
    }
    return false;
}

bool Function::Set(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.empty() || objs.size() != 2) {
        throw SyntaxError("No args");
    }
    if (Is<Object>(objs[0])) {
        return scope.Set(As<Symbol>(objs[0])->GetName(), objs[1]);
    }
    return false;
}

std::shared_ptr<Object> Function::If(Scope& scope,
                                     const std::vector<std::shared_ptr<Object>>& objs) {
    if (objs.empty()) {
        throw SyntaxError("No args");
    }
    auto solve = Solve(scope, objs[0]);

    if (!Is<Symbol>(solve)) {
        throw SyntaxError("No bool");
    }

    auto ans_solve = As<Symbol>(solve)->GetName();

    Scope scope_res = scope;

    std::shared_ptr<Object> res;

    if (ans_solve == "#t") {
        res = Solve(scope_res, objs[1]);
    } else {
        if (objs.size() < 3) {
            res = std::shared_ptr<Object>(new Symbol("()"));
        } else {
            res = Solve(scope_res, objs[2]);
        }
    }

    scope = scope_res;

    return res;
}

bool Function::SetCar(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs) {
    std::string variable;

    if (Is<Cell>(objs[0])) {
        auto solve = Solve(scope, objs[0]);
        variable = As<Symbol>(solve)->GetName();
    } else {
        variable = As<Symbol>(objs[0])->GetName();
    }

    std::stringstream ss{scope.GetValue(variable)->Serialize()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    auto new_val = As<Cell>(input_ast);

    std::shared_ptr<Object> val_first = objs[1];

    if (Is<Symbol>(objs[1])) {
        auto text = As<Symbol>(objs[1])->GetName();
        if (variable != text) {
            val_first = Solve(scope, objs[1]);
        }
    } else if (Is<Cell>(objs[1])) {
        val_first = Solve(scope, objs[1]);
    }

    new_val->SetFirst(val_first);

    std::shared_ptr<Object> val(new Symbol(new_val->Serialize()));

    scope.SetValue(variable, val);

    return true;
}

bool Function::SetCdr(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs) {
    std::string variable;

    if (Is<Cell>(objs[0])) {
        auto solve = Solve(scope, objs[0]);
        variable = As<Symbol>(solve)->GetName();
    } else {
        variable = As<Symbol>(objs[0])->GetName();
    }

    std::stringstream ss{scope.GetValue(variable)->Serialize()};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    auto new_val = As<Cell>(input_ast);

    std::shared_ptr<Object> val_second = objs[1];

    if (Is<Symbol>(objs[1])) {
        auto text = As<Symbol>(objs[1])->GetName();
        if (variable != text) {
            val_second = Solve(scope, objs[1]);
        }
    } else if (Is<Cell>(objs[1])) {
        val_second = Solve(scope, objs[1]);
    }

    new_val->SetSecond(val_second);

    std::shared_ptr<Object> val(new Symbol(new_val->Serialize()));

    scope.SetValue(variable, val);

    return true;
}
