#include <tokenizer.h>

void Tokenizer::Next() {
    if (!is_finished_stream_) {
        if (CheckEOF()) {
            is_finished_stream_ = true;
            return;
        }
        char symbol = static_cast<char>(tokenizer_->get());
        if (CheckPoint(symbol)) {
            CreateDotToken();
        } else if (CheckOpenBracket(symbol)) {
            CreateOpenBracketToken();
        } else if (CheckCloseBracket(symbol)) {
            CreateCloseBracketToken();
        } else if (CheckQuote(symbol)) {
            CreateQuoteToken();
        } else if (CheckBeginSymbol(symbol)) {
            std::string name = HandleSymbolSequence(symbol);
            if (CheckBoolean(name)) {
                CreateBooleanToken(name);
            } else {
                CreateSymbolToken(name);
            }
        } else if (CheckPlus(symbol) || CheckMinus(symbol)) {
            HandleUnarySign(symbol);
        } else if (CheckNum(symbol)) {
            std::string number = CollectNum(symbol);
            CreateConstantToken(std::stoi(number));
        } else if (CheckSpace(symbol) || CheckNextLine(symbol)) {
            Next();
        } else {
            throw SyntaxError("error in tokenizer occurred");
        }
    } else {
        throw SyntaxError("error in parser occurred");
    }
}

bool Tokenizer::IsEnd() {
    return is_finished_stream_;
}

Token Tokenizer::GetToken() {
    return last_token_;
}

void Tokenizer::CreateConstantToken(int value) {
    last_token_ = ConstantToken{value};
}

void Tokenizer::CreateSymbolToken(std::string &name) {
    last_token_ = SymbolToken{name};
}

void Tokenizer::CreateQuoteToken() {
    last_token_ = QuoteToken{};
};

void Tokenizer::CreateCloseBracketToken() {
    last_token_ = BracketToken{BracketToken::CLOSE};
}

void Tokenizer::CreateOpenBracketToken() {
    last_token_ = BracketToken{BracketToken::OPEN};
}

void Tokenizer::CreateDotToken() {
    last_token_ = DotToken{};
}

void Tokenizer::CreateBooleanToken(std::string value) {
    if (value == "#f") {
        last_token_ = BooleanToken{false};
    } else {
        last_token_ = BooleanToken{true};
    }
}

bool Tokenizer::CheckBeginSymbol(char s) {
    static const auto kBeginningParser = std::regex("[a-zA-Z<=>*/#]");
    std::string to_check;
    to_check += s;
    return std::regex_search(to_check.begin(), to_check.end(), kBeginningParser);
}

bool Tokenizer::CheckInnerSymbol(char s) {
    static const auto kInParser = std::regex("[a-zA-Z<=>*/#0-9?!-]");
    std::string to_check;
    to_check += s;
    return std::regex_search(to_check.begin(), to_check.end(), kInParser);
}

bool Tokenizer::CheckEOF() {
    return tokenizer_->peek() == EOF;
}

bool Tokenizer::CheckEOF(char s) {
    return s == EOF;
}

bool Tokenizer::CheckNum(char s) {
    return std::isalnum(s);
}

bool Tokenizer::CheckNextLine(char s) {
    return s == NextLine;
}

bool Tokenizer::CheckOpenBracket(char s) {
    return s == OpenBracket;
}

bool Tokenizer::CheckCloseBracket(char s) {
    return s == CloseBracket;
}

bool Tokenizer::CheckPlus(char s) {
    return s == Plus;
}

bool Tokenizer::CheckMinus(char s) {
    return s == Minus;
}

bool Tokenizer::CheckPoint(char s) {
    return s == Dot;
}

bool Tokenizer::CheckQuote(char s) {
    return s == Quote;
}

bool Tokenizer::CheckSpace(char s) {
    return s == Space;
}

bool Tokenizer::CheckBoolean(std::string &s) {
    return s == "#f" || s == "#t";
}

std::string Tokenizer::CollectNum(char symbol) {
    std::string num;
    num += symbol;
    while (!CheckEOF(symbol) && CheckNum(symbol)) {
        if (CheckNum(tokenizer_->peek())) {
            symbol = tokenizer_->get();
            num += symbol;
        } else {
            break;
        }
    }
    return num;
}

std::string Tokenizer::HandleSymbolSequence(char symbol) {
    std::string name;
    while (!CheckEOF(symbol) && (CheckBeginSymbol(symbol) || CheckInnerSymbol(symbol))) {
        name += symbol;
        symbol = tokenizer_->peek();
        if (!CheckBeginSymbol(symbol) && !CheckInnerSymbol(symbol)) {
            break;
        }
        symbol = tokenizer_->get();
    }
    return name;
}

void Tokenizer::HandleUnarySign(char symbol) {
    char next_symbol;
    next_symbol = tokenizer_->peek();
    if (!CheckEOF(next_symbol) && CheckNum(next_symbol)) {
        next_symbol = tokenizer_->get();
        int number = std::stoi(CollectNum(next_symbol));
        if (symbol == '-') {
            number *= -1;
        }
        CreateConstantToken(number);
    } else {
        std::string name;
        name += symbol;
        CreateSymbolToken(name);
    }
}