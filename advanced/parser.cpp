#include "parser.h"

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    Token token = tokenizer->GetToken();
    tokenizer->Next();
    if (token == Token{BracketToken::OPEN}) {
        return ReadList(tokenizer);
    } else if (token == Token{QuoteToken{}}) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Not Bad");
        }
        auto quote = std::shared_ptr<Object>(new Symbol("\'"));
        auto cell = std::make_shared<Cell>(quote);
        auto text = Read(tokenizer);
        if (tokenizer->GetToken() == Token{BracketToken::CLOSE} || tokenizer->IsEnd()) {
            cell->SetSecond(text);
        } else {
            auto second_cell = std::make_shared<Cell>(text);
            cell->SetSecond(second_cell);
        }
        return std::shared_ptr<Object>(cell);
    } else if (ConstantToken* x = std::get_if<ConstantToken>(&token)) {
        return std::shared_ptr<Object>(new Number(x->value));
    } else if (SymbolToken* x = std::get_if<SymbolToken>(&token)) {
        return std::shared_ptr<Object>(new Symbol(x->name));
    } else if (token == Token{QuoteToken{}} && tokenizer->IsEnd()) {
        throw SyntaxError("Not Bad");
    }
    return nullptr;
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    auto cell = std::make_shared<Cell>();
    auto obj = std::shared_ptr<Object>(cell);
    Token token = tokenizer->GetToken();
    bool is_pair = false, is_quote = false;
    if (SymbolToken* x = std::get_if<SymbolToken>(&token)) {
        if (x->name == "quote") {
            is_quote = true;
        }
    }
    if (token == Token{BracketToken::CLOSE}) {
        Read(tokenizer);
        return nullptr;
    }
    while (token != Token{BracketToken::CLOSE}) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Where is your skobka?");
        } else if (token == Token{QuoteToken{}}) {
            cell->SetSecond(Read(tokenizer));
            while (Is<Cell>(cell->GetSecond())) {
                cell = As<Cell>(cell->GetSecond());
            }
        } else if (token == Token{DotToken{}}) {
            if (!cell->GetFirst()) {
                throw SyntaxError("where is your first?");
            }
            is_pair = true;
            Read(tokenizer);
            if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
                throw SyntaxError("Where is your second1?");
            }
        } else {
            auto read_token = Read(tokenizer);
            if (!is_pair && !cell->GetFirst()) {
                cell->SetFirst(read_token);
            } else if (!is_pair && cell->GetFirst() && !is_quote) {
                auto second_cell = std::make_shared<Cell>(read_token);
                cell->SetSecond(second_cell);
                cell = second_cell;
            } else {
                if (cell->GetSecond()) {
                    throw SyntaxError("You already have a second.");
                }
                cell->SetSecond(read_token);
            }
            if (tokenizer->IsEnd()) {
                throw SyntaxError("Where is your skobka2?");
            }
        }
        token = tokenizer->GetToken();
    }
    Read(tokenizer);
    return obj;
}
