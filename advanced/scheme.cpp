#include "scheme.h"
#include "function.h"
#include <sstream>

std::vector<std::shared_ptr<Object>> Convert(Scope& scope, const std::shared_ptr<Object>& ast) {
    std::vector<std::shared_ptr<Object>> res;

    if (!ast) {
        return res;
    }

    auto cell = As<Cell>(ast);

    if (!cell && ast) {
        res.push_back(ast);
        return res;
    }

    if (!cell->GetFirst()) {
        throw RuntimeError("Convert args error");
    }

    do {
        if (Is<Number>(cell->GetFirst())) {
            res.push_back(cell->GetFirst());
        } else {
            std::string for_quote;
            if (Is<Symbol>(cell->GetFirst())) {
                for_quote = As<Symbol>(cell->GetFirst())->GetName();
            }
            if (for_quote == "\'" || for_quote == "quote") {
                auto sub_ast = As<Object>(cell);
                auto solve = Solve(scope, sub_ast);
                res.push_back(solve);
                if (cell->GetSecond()) {
                    cell = As<Cell>(cell->GetSecond());
                    while (cell) {
                        if (Is<Symbol>(cell->GetFirst())) {
                            auto check = As<Symbol>(cell->GetFirst())->GetName();
                            if (check == "\'" || check == "quote") {
                                sub_ast = As<Object>(cell);
                                solve = Solve(scope, sub_ast);
                                res.push_back(solve);
                                return res;
                            }
                            cell = As<Cell>(cell->GetSecond());
                        } else {
                            return res;
                        }
                    }
                    return res;
                }
            } else if (Is<Symbol>(cell->GetFirst())) {
                res.push_back(Solve(scope, cell->GetFirst()));
            } else {
                auto sub_ast = As<Object>(cell->GetFirst());
                auto solve = Solve(scope, sub_ast);
                if (Is<Lambda>(solve)) {
                    auto lambda = As<Lambda>(solve);
                    if (lambda->plug) {
                        solve = Solve(scope, cell);
                    }
                }
                res.push_back(solve);
            }
        }
        if (!cell->GetSecond()) {
            return res;
        }
        if (cell && cell->GetSecond()) {
            cell = As<Cell>(cell->GetSecond());
            if (!cell) {
                return res;
            }
        } else if (!cell) {
            return res;
        } else {
            break;
        }
    } while (As<Cell>(cell->GetSecond()) || As<Cell>(cell->GetFirst()));

    if (!Is<Cell>(cell->GetFirst())) {
        auto solve = Solve(scope, cell->GetFirst());
        if (cell->GetFirst() != res.front()) {
            if (Is<Symbol>(cell->GetFirst())) {
                auto text = As<Symbol>(cell->GetFirst())->GetName();
                if (text == "'" || text == "quote") {
                    return res;
                }
            }
            res.push_back(solve);
        }
    }

    return res;
}

std::vector<std::shared_ptr<Object>> ConvertIf(Scope& scope, const std::shared_ptr<Object>& ast) {
    std::vector<std::shared_ptr<Object>> res;

    auto cell = As<Cell>(ast);

    while (cell) {
        res.push_back(cell->GetFirst());

        cell = As<Cell>(cell->GetSecond());
    }

    return res;
}

std::shared_ptr<Object> Solve(Scope& scope, const std::shared_ptr<Object>& ast) {
    if (Is<Symbol>(ast)) {
        auto obj = As<Symbol>(ast);
        if (obj->GetName() != "#t" && obj->GetName() != "#f") {
            auto val = scope.GetValue(obj->GetName());
            if (val) {
                return val;
            } else {
                throw NameError("No name");
            }
        }
    }

    if (!Is<Cell>(ast)) {
        return ast;
    }

    if (!ast) {
        throw RuntimeError("No AST");
    }
    std::shared_ptr<Cell> root = As<Cell>(ast);

    auto function_name = root->GetFirst();

    if (!function_name) {
        throw RuntimeError("");
    }

    if (!Is<Symbol>(function_name)) {
        if (Is<Cell>(function_name)) {
            auto lambda = As<Cell>(function_name)->GetFirst();
            if (Is<Symbol>(lambda)) {
                auto check_lambda = As<Symbol>(lambda)->GetName();
                if (check_lambda == "lambda") {
                    auto lambda_var = ConvertIf(
                        scope, As<Cell>(As<Cell>(function_name)->GetSecond())->GetFirst());
                    auto lambda_exp = ConvertIf(
                        scope, As<Cell>(As<Cell>(function_name)->GetSecond())->GetSecond());
                    auto lambda_args = Convert(scope, root->GetSecond());

                    Function lambda_func;

                    bool flag = false;

                    for (const auto& v : lambda_var) {
                        if (!Is<Symbol>(v)) {
                            flag = true;
                            break;
                        }
                    }

                    if ((!lambda_args.empty() && Is<Lambda>(lambda_args[0])) || flag) {
                        lambda_exp =
                            ConvertIf(scope, As<Cell>(As<Cell>(function_name)->GetSecond()));
                        lambda_args = Convert(scope, root->GetSecond());

                        return lambda_func.LambdaFunc(scope, {}, lambda_exp, {});
                    }

                    if (lambda_exp.empty()) {
                        throw SyntaxError("lambda fail");
                    }

                    return lambda_func.LambdaFunc(scope, lambda_var, lambda_exp, lambda_args);
                }
            }
        }
        if (Is<Cell>(function_name)) {
            if (As<Cell>(function_name)->GetSecond()) {
                return Solve(scope, function_name);
            }
        }
        throw NameError("maybe you lost");
    }

    auto function = Function(As<Symbol>(function_name)->GetName());

    auto check_args = As<Cell>(root->GetSecond());

    if (function.GetName() == "quote" || function.GetName() == "\'") {
        if (!check_args && !root->GetSecond()) {
            return std::shared_ptr<Object>(new Symbol("()"));
        }
        return std::shared_ptr<Object>(new Symbol(root->GetSecond()->Serialize()));
    }

    std::vector<std::shared_ptr<Object>> args;

    auto func_name = function.GetName();

    if (func_name == "define" || func_name == "set!") {
        if (!check_args) {
            throw SyntaxError("No args");
        }
        if (Is<Cell>(As<Cell>(root->GetSecond())->GetFirst())) {
            args.push_back(As<Cell>(As<Cell>(root->GetSecond())->GetFirst())->GetFirst());

            std::shared_ptr<Cell> lambda_body(
                new Cell(As<Cell>(As<Cell>(root->GetSecond())->GetFirst())->GetSecond()));
            lambda_body->SetSecond(As<Cell>(root->GetSecond())->GetSecond());

            std::shared_ptr<Cell> lambda_name(
                new Cell(std::shared_ptr<Symbol>(new Symbol("lambda"))));
            lambda_name->SetSecond(lambda_body);

            std::shared_ptr<Cell> lambda_root(new Cell(lambda_name));

            auto args_for_define = Convert(scope, lambda_root);
            args.insert(args.end(), args_for_define.begin(), args_for_define.end());
        } else {
            args.push_back(As<Cell>(root->GetSecond())->GetFirst());
            auto args_for_define = Convert(scope, As<Cell>(root->GetSecond())->GetSecond());
            args.insert(args.end(), args_for_define.begin(), args_for_define.end());
        }
    } else if (func_name == "if") {
        args = ConvertIf(scope, root->GetSecond());
    } else if (func_name == "set-car!" || func_name == "set-cdr!") {
        if (!check_args) {
            throw SyntaxError("No args");
        }
        args.push_back(As<Cell>(root->GetSecond())->GetFirst());
        auto args_for_define = ConvertIf(scope, As<Cell>(root->GetSecond())->GetSecond());
        args.insert(args.end(), args_for_define.begin(), args_for_define.end());
    } else if (func_name == "lambda") {
        return std::shared_ptr<Object>(new Lambda(true));
    } else if (root->GetSecond()) {
        args = Convert(scope, root->GetSecond());
    }

    return function.Apply(scope, args);
}

std::string Interpreter::Run(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    auto input_ast = Read(&tokenizer);

    if (!tokenizer.IsEnd()) {
        throw SyntaxError("");
    }

    if (!input_ast || (Is<Cell>(input_ast) && Is<Number>(As<Cell>(input_ast)->GetFirst()))) {
        throw RuntimeError("not today, bro");
    }

    if (Is<Symbol>(input_ast)) {
        auto obj = As<Symbol>(input_ast);
        if (obj->GetName() != "#t" && obj->GetName() != "#f") {
            auto val = global_scope_.GetValue(obj->GetName());
            if (!val) {
                throw NameError("No name");
            }
            return val->Serialize();
        }
    }

    auto lambda_check = As<Cell>(input_ast);
    if (lambda_check && Is<Symbol>(lambda_check->GetFirst())) {
        if (As<Symbol>(lambda_check->GetFirst())->GetName() == "lambda") {
            if (!lambda_check->GetSecond()) {
                throw SyntaxError("");
            }
        }
    }

    auto solve = Solve(global_scope_, input_ast);

    if (Is<Lambda>(solve)) {
        throw SyntaxError("");
    }

    return solve->Serialize();
}
