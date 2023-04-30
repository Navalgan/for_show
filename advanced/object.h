#pragma once

#include "error.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <sstream>
#include <memory>
#include <unordered_map>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::shared_ptr<Object> Eval() = 0;
    virtual std::string Serialize() = 0;
    virtual ~Object() = default;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return As<T>(obj) != nullptr;
}

class Number : public Object {
public:
    Number(int value);

    Number(const std::string& value);

    std::shared_ptr<Object> Eval() override;

    std::string Serialize() override;

    int64_t GetValue() const;

private:
    int64_t value_;
};

class Symbol : public Object {
public:
    Symbol(std::string_view str);

    std::shared_ptr<Object> Eval() override;

    std::string Serialize() override;

    const std::string& GetName() const;

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell();

    Cell(const std::shared_ptr<Object>& first);

    std::shared_ptr<Object> Eval() override;

    std::string Serialize() override;

    void SetFirst(const std::shared_ptr<Object>& first);

    void SetSecond(const std::shared_ptr<Object>& second);

    std::shared_ptr<Object> GetFirst() const;

    std::shared_ptr<Object> GetSecond() const;

    std::shared_ptr<Object> GetSecond();

private:
    std::shared_ptr<Object> first_, second_;
};

class Scope {
public:
    std::shared_ptr<Object> GetValue(const std::string& str);

    bool SetValue(const std::string& str, std::shared_ptr<Object> obj);

    bool Set(const std::string& str, std::shared_ptr<Object> obj);

    std::unordered_map<std::string, std::shared_ptr<Object>> GetScope();

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> scope_;
};

class Lambda : public Object {
public:
    Lambda();

    Lambda(bool flag);

    Lambda(Scope& scope, const std::vector<std::shared_ptr<Object>>& var,
           const std::vector<std::shared_ptr<Object>>& exp);

    std::shared_ptr<Object> Eval() override;

    std::string Serialize() override;

    std::shared_ptr<Object> Run(Scope& scope, const std::vector<std::shared_ptr<Object>>& args);

    Scope& GetScope();

    void SetScope(Scope& scope);

    bool plug = false;

private:
    std::vector<std::shared_ptr<Object>> variables_;
    std::vector<std::shared_ptr<Object>> expressions_;
    Scope scope_;
};
