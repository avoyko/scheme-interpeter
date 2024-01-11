#include "scheme.h"

std::string Interpreter::Run(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    auto ast = Read(&tokenizer);
    auto final_ast = Unpack<Object>(ast);
    return PerformOutput(final_ast);
}

void Interpreter::Serialize(std::shared_ptr<Object> ast, std::string& ans) {
    if (Is<Number>(ast)) {
        ans += std::to_string(As<Number>(ast)->GetValue());
    } else if (Is<Boolean>(ast)) {
        if (As<Boolean>(ast)->GetValue()) {
            ans += "#t";
        } else {
            ans += "#f";
        }
    } else if (Is<Symbol>(ast)) {
        ans += As<Symbol>(ast)->GetName();
    } else if (Is<Cell>(ast)) {
        std::shared_ptr<Object> first = As<Cell>(ast)->GetFirst();
        std::shared_ptr<Object> second = As<Cell>(ast)->GetSecond();
        if (!Is<Cell>(first) && !Is<Cell>(second) && second) {
            Serialize(first, ans);
            ans += " . ";
            Serialize(second, ans);
        } else {
            if (Is<Cell>(first) || !first) {
                ans += '(';
                Serialize(first, ans);
                ans += ')';
            } else {
                Serialize(first, ans);
            }
            if (second) {
                ans += ' ';
                Serialize(second, ans);
            }
        }
    }
}

std::string Interpreter::PerformOutput(std::shared_ptr<Object> ast) {
    std::string ans;
    if (Is<Cell>(ast) || ast == nullptr) {
        ans += '(';
        Serialize(ast, ans);
        ans += ')';
    } else {
        Serialize(ast, ans);
    }
    return ans;
}
