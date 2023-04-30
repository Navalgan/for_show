#include "tokenizer.h"
#include "error.h"

bool IsSymbolBegin(char ch) {
    if (islower(ch) || isupper(ch)) {
        return true;
    }
    static const std::vector<char> kChars{'<', '=', '>', '*', '/', '#'};
    for (const auto sym : kChars) {
        if (ch == sym) {
            return true;
        }
    }
    return false;
}

bool IsSymbolInside(char ch) {
    if (IsSymbolBegin(ch) || isdigit(ch)) {
        return true;
    }
    static const std::vector<char> kChars{'?', '!', '-'};
    for (const auto sym : kChars) {
        if (ch == sym) {
            return true;
        }
    }
    return false;
}

bool SymbolToken::operator==(const SymbolToken &other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}

bool DotToken::operator==(const DotToken &) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken &other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream *in) : is_end_(false), in_(*in), last_token_(QuoteToken{}) {
    ch_ = in_.get();
    if (ch_ == '\0' && in_.eof()) {
        throw SyntaxError("Not Bad");
    }
    Next();
}

bool Tokenizer::IsEnd() {
    return is_end_;
}

void Tokenizer::Next() {
    if (in_.eof()) {
        is_end_ = true;
    }
    while (isspace(ch_)) {
        ch_ = in_.get();
        if (in_.eof()) {
            is_end_ = true;
        }
    }
    if (ch_ == '.') {
        last_token_ = DotToken{};
    } else if (ch_ == '\'') {
        last_token_ = QuoteToken{};
    } else if (ch_ == '(') {
        last_token_ = BracketToken::OPEN;
    } else if (ch_ == ')') {
        last_token_ = BracketToken::CLOSE;
    } else if (ch_ == '+' || ch_ == '-') {
        char sign = ch_;
        ch_ = in_.get();
        if (isdigit(ch_)) {
            ReadDigit(sign);
        } else {
            last_token_ = SymbolToken{std::string(1, sign)};
        }
        return;
    } else if (isdigit(ch_)) {
        ReadDigit('+');
        return;
    } else if (IsSymbolBegin(ch_)) {
        std::string cur_token;
        do {
            cur_token += ch_;
            ch_ = in_.get();
        } while (!in_.eof() && IsSymbolInside(ch_));
        last_token_ = SymbolToken{cur_token};
        return;
    }
    static const std::vector<char> kChars{':', ';', '?', '@', '\"', '&', '$', ',', '\\'};
    for (const auto sym : kChars) {
        if (ch_ == sym) {
            throw SyntaxError("Not Bad");
        }
    }
    ch_ = in_.get();
}

Token Tokenizer::GetToken() {
    return last_token_;
}

void Tokenizer::ReadDigit(char sign) {
    std::string cur_token;
    do {
        cur_token += ch_;
        ch_ = in_.get();
    } while (!in_.eof() && isdigit(ch_));
    int s = sign == '+' ? 1 : -1;
    last_token_ = ConstantToken{atoi(cur_token.c_str()) * s};
}
