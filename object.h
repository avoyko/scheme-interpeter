#pragma once

#include <memory>
#include <tokenizer.h>
#include <unordered_map>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) {
        throw RuntimeError("Not implemented method within this object");
    };
    virtual std::shared_ptr<Object> EvalToFunc() {
        throw RuntimeError("Not implemented method within this object");
    }

    virtual ~Object() = default;
};

typedef std::vector<std::shared_ptr<Object>> FuncArgs;

class Number : public Object {
public:
    Number(ConstantToken const_token) : value_(const_token.value){};
    std::shared_ptr<Object> EvalToFunc() override;
    int GetValue() const;

private:
    int value_;
};

class Boolean : public Object {
public:
    Boolean(BooleanToken bool_token) : value_(bool_token.value){};
    bool GetValue() const;

private:
    bool value_;
};

class Symbol : public Object {
private:
    friend class OperationsMap;

public:
    Symbol(SymbolToken symbol_token) : name_(symbol_token.name){};
    const std::string &GetName() const;
    std::shared_ptr<Object> EvalToFunc() override;

private:
    std::string name_;
};

class Quote : public Object {
public:
    Quote(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class Cell : public Object {
public:
    Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second)
            : first_(first), second_(second){};

    std::shared_ptr<Object> GetFirst() const;
    std::shared_ptr<Object> GetSecond() const;
    void SetFirst(std::shared_ptr<Object> other_first);
    void SetSecond(std::shared_ptr<Object> other_second);

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

template <typename Functor>
class Calculate : public Object {
public:
    Calculate(){};
    std::shared_ptr<Object> Apply(FuncArgs &args) override;
    int SetDefaultValue(FuncArgs &args);
    size_t SetStartIndex();
};

class Absolute : public Object {
public:
    Absolute(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class And : public Object {
public:
    And(){};
    std::shared_ptr<Object> Apply(FuncArgs &args) override;
};

class Or : public Object {
public:
    Or(){};
    std::shared_ptr<Object> Apply(FuncArgs &args) override;
};

class Not : public Object {
public:
    Not(){};
    std::shared_ptr<Object> Apply(FuncArgs &args) override;
};

template <typename Functor>
class Monotony : public Object {
public:
    Monotony(){};
    std::shared_ptr<Object> Apply(FuncArgs &args) override;
    bool Check(Functor functor, int first, int second);
};

template <typename T>
class Predicate : public Object {
public:
    Predicate(){};
    std::shared_ptr<Object> Apply(FuncArgs &args) override;
};

template <typename Functor>
class ListPredicate : public Object {
public:
    ListPredicate(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class Cons : public Object {
public:
    Cons(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class Car : public Object {
public:
    Car(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class Cdr : public Object {
public:
    Cdr(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class MakeList : public Object {
public:
    MakeList(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class ListRef : public Object {
public:
    ListRef(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

class ListTail : public Object {
public:
    ListTail(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> &args) override;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object> &obj) {
    if (obj == nullptr) {
        throw RuntimeError("Invalid cast");
    }
    return std::static_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object> &obj) {
    auto ptr = std::dynamic_pointer_cast<T>(obj);
    return ptr != nullptr;
}

template <typename T>
class Max {
public:
    Max(){};
    auto operator()(T first, T second) {
        return std::max(first, second);
    }
};

template <typename T>
class Min {
public:
    Min(){};
    auto operator()(T first, T second) {
        return std::min(first, second);
    }
};

class NullFunc {
public:
    NullFunc(){};
    auto operator()(std::shared_ptr<Object> obj) {
        if (Is<Cell>(obj) && Is<Quote>(As<Cell>(obj)->GetFirst())) {
            obj = As<Cell>(obj)->GetSecond();
        }
        return obj == nullptr;
    }
};

class PairFunc {
public:
    PairFunc(){};
    auto operator()(std::shared_ptr<Object> obj) {
        if (Is<Cell>(obj) && Is<Quote>(As<Cell>(obj)->GetFirst())) {
            obj = As<Cell>(obj)->GetSecond();
        }
        if (Is<Cell>(obj)) {
            auto first = As<Cell>(obj)->GetFirst();
            auto second = As<Cell>(obj)->GetSecond();
            return !Is<Cell>(first) &&
                   (!Is<Cell>(second) || As<Cell>(second)->GetSecond() == nullptr);
        }
        return false;
    }
};

class ListFunc {
public:
    ListFunc(){};
    auto operator()(std::shared_ptr<Object> obj) {
        if (Is<Cell>(obj) && Is<Quote>(As<Cell>(obj)->GetFirst())) {
            obj = As<Cell>(obj)->GetSecond();
        }
        if (Is<Cell>(obj)) {
            std::shared_ptr<Object> first = As<Cell>(obj)->GetFirst();
            std::shared_ptr<Object> second = As<Cell>(obj)->GetSecond();
            if (IsPairCell(first, second)) {
                return false;
            }
            while (Is<Cell>(second)) {
                first = As<Cell>(second)->GetFirst();
                second = As<Cell>(second)->GetSecond();
                if (IsPairCell(first, second)) {
                    return false;
                }
            }
            return true;
        }
        return obj == nullptr;
    }

    bool IsPairCell(std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        return !Is<Cell>(first) && !Is<Cell>(second) && second != nullptr;
    }
};

using LessFunc = std::less<int>;
using GreaterFunc = std::greater<int>;
using GreaterEqualFunc = std::greater_equal<int>;
using LessEqualFunc = std::less_equal<int>;
using EqualFunc = std::equal_to<int>;
using PlusFunc = std::plus<int>;
using MinusFunc = std::minus<int>;
using MultFunc = std::multiplies<int>;
using DivFunc = std::divides<int>;
using MaxFunc = Max<int>;
using MinFunc = Min<int>;

class OperationsMap {
public:
    static OperationsMap &Instantiate() {
        static OperationsMap map;
        return map;
    }

    auto GetOperation(std::string &name) {
        return operations[name];
    }

    bool CheckExistence(std::string &name) {
        return operations.find(name) != operations.end();
    }

    std::map<std::string, std::shared_ptr<Object>> operations{
            {"+", std::make_shared<Calculate<PlusFunc>>()},
            {"-", std::make_shared<Calculate<MinusFunc>>()},
            {"*", std::make_shared<Calculate<MultFunc>>()},
            {"/", std::make_shared<Calculate<DivFunc>>()},
            {"max", std::make_shared<Calculate<MaxFunc>>()},
            {"min", std::make_shared<Calculate<MinFunc>>()},
            {"abs", std::make_shared<Absolute>()},
            {"or", std::make_shared<Or>()},
            {"and", std::make_shared<And>()},
            {"not", std::make_shared<Not>()},
            {"boolean?", std::make_shared<Predicate<Boolean>>()},
            {"number?", std::make_shared<Predicate<Number>>()},
            {"null?", std::make_shared<ListPredicate<NullFunc>>()},
            {"pair?", std::make_shared<ListPredicate<PairFunc>>()},
            {"list?", std::make_shared<ListPredicate<ListFunc>>()},
            {"=", std::make_shared<Monotony<EqualFunc>>()},
            {">", std::make_shared<Monotony<GreaterFunc>>()},
            {"<", std::make_shared<Monotony<LessFunc>>()},
            {"<=", std::make_shared<Monotony<LessEqualFunc>>()},
            {">=", std::make_shared<Monotony<GreaterEqualFunc>>()},
            {"quote", std::make_shared<Quote>()},
            {"cons", std::make_shared<Cons>()},
            {"car", std::make_shared<Car>()},
            {"cdr", std::make_shared<Cdr>()},
            {"list-ref", std::make_shared<ListRef>()},
            {"list-tail", std::make_shared<ListTail>()},
            {"list", std::make_shared<MakeList>()},
    };
};
