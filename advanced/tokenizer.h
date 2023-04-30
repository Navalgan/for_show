#pragma once

#include <cstdlib>
#include <variant>
#include <vector>
#include <optional>
#include <istream>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    void ReadDigit(char sign);

    char ch_;
    bool is_end_;
    std::istream& in_;
    Token last_token_;
};
