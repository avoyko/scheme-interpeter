#pragma once

#include <memory>
#include <parser.h>
#include <object.h>

bool CheckNumToken(Token token) {
    return std::holds_alternative<ConstantToken>(token);
}

bool CheckSymbolToken(Token token) {
    return std::holds_alternative<SymbolToken>(token);
}

bool CheckDotToken(Token token) {
    return std::holds_alternative<DotToken>(token);
}

bool CheckOpenBracketToken(Token token) {
    return token == Token{BracketToken::OPEN};
}

bool CheckCloseBracketToken(Token token) {
    return token == Token{BracketToken::CLOSE};
}

bool CheckQuoteToken(Token token) {
    return std::holds_alternative<QuoteToken>(token);
}

bool CheckBooleanToken(Token token) {
    return std::holds_alternative<BooleanToken>(token);
}

std::shared_ptr <Object> ReadOne(Tokenizer *tokenizer);

std::shared_ptr <Object> ReadList(Tokenizer *tokenizer) {

    std::shared_ptr <Object> first;
    std::shared_ptr <Object> second;
    Token current_token = tokenizer->GetToken();

    if (CheckOpenBracketToken(current_token)) {
        first = ReadOne(tokenizer);
        second = ReadList(tokenizer);
        return std::make_shared<Cell>(first, second);
    }
    if (CheckDotToken(current_token)) {
        tokenizer->Next();
        first = ReadOne(tokenizer);
        return first;
    }

    if (!CheckCloseBracketToken(current_token)) {
        first = ReadOne(tokenizer);
        second = ReadList(tokenizer);
        return std::make_shared<Cell>(first, second);
    }
    return nullptr;
}

std::shared_ptr <Object> ReadOne(Tokenizer *tokenizer) {
    if (!tokenizer->IsEnd()) {
        Token current_token = tokenizer->GetToken();
        std::shared_ptr <Object> ptr;
        if (CheckNumToken(current_token)) {
            Number num(std::get<ConstantToken>(current_token));
            ptr = std::make_shared<Number>(num);
        } else if (CheckSymbolToken(current_token)) {
            Symbol symbol(std::get<SymbolToken>(current_token));
            ptr = std::make_shared<Symbol>(symbol);
        } else if (CheckOpenBracketToken(current_token)) {
            tokenizer->Next();
            if (CheckDotToken(tokenizer->GetToken())) {
                throw SyntaxError("error in parser occurred");
            }
            ptr = ReadList(tokenizer);
        } else if (CheckDotToken(current_token)) {
            throw SyntaxError("error in parser occurred");
        } else if (CheckQuoteToken(current_token)) {
            tokenizer->Next();
            return std::make_shared<Cell>(std::make_shared<Quote>(), ReadOne(tokenizer));
        } else if (CheckBooleanToken(current_token)) {
            BooleanToken boolean(std::get<BooleanToken>(current_token));
            ptr = std::make_shared<Boolean>(boolean);
        }
        tokenizer->Next();
        return ptr;
    } else {
        throw SyntaxError("error in parser occurred");
    }
}

std::shared_ptr <Object> Read(Tokenizer *tokenizer) {
    std::shared_ptr <Object> ast = ReadOne(tokenizer);
    if (!tokenizer->IsEnd() || Is<Quote>(ast)) {
        throw SyntaxError("error in parser occurred");
    }
    return ast;
}
