#pragma once

#include <vector>
#include <memory>
#include <object.h>

template <typename Obj>
void GetElems(std::shared_ptr<Obj> tree, std::vector<std::shared_ptr<Obj>> &container);

template <typename Obj>
std::shared_ptr<Obj> Unpack(std::shared_ptr<Obj> ast) {
    if (Is<Cell>(ast)) {
        std::shared_ptr<Obj> first = As<Cell>(ast)->GetFirst();
        std::shared_ptr<Obj> second = As<Cell>(ast)->GetSecond();
        if (Is<Symbol>(first)) {
            std::shared_ptr<Obj> func;
            func = first->EvalToFunc();
            if (func) {
                std::vector<std::shared_ptr<Obj>> args;
                GetElems(second, args);
                return func->Apply(args);
            }
            return ast;
        } else if (Is<Quote>(first)) {
            return second;
        } else {
            throw RuntimeError("Cannot evaluate number to function");
        }
    } else {
        if (!ast) {
            throw RuntimeError("Nothing to unpack");
        }
        return ast;
    }
}

template <typename Obj>
void GetElems(std::shared_ptr<Obj> tree, std::vector<std::shared_ptr<Obj>> &container) {
    if (Is<Cell>(tree)) {
        std::shared_ptr<Obj> first = As<Cell>(tree)->GetFirst();
        std::shared_ptr<Obj> second = As<Cell>(tree)->GetSecond();
        if (!Is<Quote>(first)) {
            container.push_back(first);
        }
        if (Is<Cell>(second)) {
            GetElems(second, container);
        }
    } else {
        if (tree) {
            container.push_back(tree);
        }
        return;
    }
}

template <typename T, typename Obj>
bool IsTypes(std::vector<std::shared_ptr<Obj>> &to_check) {
    for (auto &elem : to_check) {
        std::shared_ptr<Obj> element = elem;
        if (Is<Cell>(elem)) {
            elem = Unpack(elem);
        }
        auto ptr = std::dynamic_pointer_cast<T>(elem);
        if (!ptr) {
            return false;
        }
    }
    return true;
}
