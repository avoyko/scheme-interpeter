#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <regex>
#include <error.h>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    };
};

struct QuoteToken {
    char s;
    bool operator==(const QuoteToken&) const {
        return true;
    };
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    };
};

enum class BracketToken { OPEN, CLOSE };

struct BooleanToken {
    bool value;
    bool operator==(const BooleanToken& other) const {
        return value == other.value;
    };
};

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    };
};

using Token =
        std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BooleanToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in) : tokenizer_(in), is_finished_stream_(false) {
        Next();
    };

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    void CreateConstantToken(int value);

    void CreateOpenBracketToken();

    void CreateCloseBracketToken();

    void CreateBooleanToken(std::string value);

    void CreateSymbolToken(std::string& name);

    void CreateQuoteToken();

    void CreateDotToken();

    bool CheckBeginSymbol(char s);

    bool CheckInnerSymbol(char s);

    bool CheckNum(char s);

    bool CheckEOF();

    bool CheckEOF(char s);

    bool CheckPlus(char s);

    bool CheckMinus(char s);

    bool CheckSpace(char s);

    bool CheckQuote(char s);

    bool CheckQuote(std::string s);

    bool CheckNextLine(char s);

    bool CheckPoint(char s);

    bool CheckOpenBracket(char s);

    bool CheckCloseBracket(char s);

    bool CheckBoolean(std::string& s);

    void HandleUnarySign(char symbol);

    std::string HandleSymbolSequence(char symbol);

    std::string CollectNum(char symbol);

private:
    std::istream* tokenizer_;
    Token last_token_;
    bool is_finished_stream_;

    enum SymbolStore : char {
        Space = ' ',
        OpenBracket = '(',
        CloseBracket = ')',
        Dot = '.',
        Quote = '\'',
        Plus = '+',
        Minus = '-',
        NextLine = '\n',
    };
};
