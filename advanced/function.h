#pragma once

#include "object.h"

class Function : public Object {
public:
    Function();

    Function(std::string_view str);

    std::shared_ptr<Object> Apply(Scope& scope, const std::vector<std::shared_ptr<Object>>& args);

    std::string GetName();

    std::shared_ptr<Object> LambdaFunc(Scope& scope,
                                       const std::vector<std::shared_ptr<Object>>& var,
                                       const std::vector<std::shared_ptr<Object>>& exp,
                                       const std::vector<std::shared_ptr<Object>>& args);

private:
    std::shared_ptr<Object> Eval() override;

    std::string Serialize() override;

    //// Arithmetic's functions

    bool IsNumber(const std::shared_ptr<Object>& obj);

    template <class Functor>
    bool Comparisons(const Functor& func, const std::vector<std::shared_ptr<Object>>& objs) {
        if (objs.empty()) {
            return true;
        }
        for (size_t i = 0; i < objs.size() - 1; ++i) {
            if (!Is<Number>(objs[i + 1]) || !Is<Number>(objs[i + 1])) {
                throw RuntimeError("Not number");
            }
            if (!func(As<Number>(objs[i])->GetValue(), As<Number>(objs[i + 1])->GetValue())) {
                return false;
            }
        }
        return true;
    }

    template <class Functor>
    std::shared_ptr<Object> Arithmetic(Functor func,
                                       const std::vector<std::shared_ptr<Object>>& objs) {
        if (!Is<Number>(objs[0])) {
            throw RuntimeError("Need number");
        }

        int64_t res = As<Number>(objs[0])->GetValue();

        for (size_t i = 1; i < objs.size(); ++i) {
            if (Is<Number>(objs[i])) {
                res = func(res, As<Number>(objs[i])->GetValue());
            } else {
                throw RuntimeError("No number");
            }
        }

        return std::shared_ptr<Object>(new Number(res));
    }

    std::shared_ptr<Object> DoubleM(std::string_view str,
                                    const std::vector<std::shared_ptr<Object>>& objs);

    std::shared_ptr<Object> Abs(const std::shared_ptr<Object>& obj);

    //// Boolean's functions

    bool IsBoolean(const std::shared_ptr<Object>& obj);

    bool Not(const std::vector<std::shared_ptr<Object>>& objs);

    void And(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs);

    void Or(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs);

    //// List's functions

    bool IsPair(const std::shared_ptr<Object>& obj);

    bool IsNull(const std::shared_ptr<Object>& obj);

    bool IsList(const std::shared_ptr<Object>& obj);

    std::shared_ptr<Object> Cons(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs);

    std::shared_ptr<Object> Car(Scope& scope, const std::shared_ptr<Object>& obj);

    std::shared_ptr<Object> Cdr(Scope& scope, const std::shared_ptr<Object>& obj);

    void List(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs);

    void ListRef(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs);

    void ListTail(std::string& answer, const std::vector<std::shared_ptr<Object>>& objs);

    //// Symbol's functions

    bool IsSymbol(const std::shared_ptr<Object>& obj);

    bool Define(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs);

    bool Set(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs);

    bool SetCar(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs);

    bool SetCdr(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs);

    //// Control_flow's functions

    std::shared_ptr<Object> If(Scope& scope, const std::vector<std::shared_ptr<Object>>& objs);

    //// Fields of class

    std::string name_of_func_;
};
