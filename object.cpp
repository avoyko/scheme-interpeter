#include <object.h>
#include <helpers.h>

std::shared_ptr<Object> Number::EvalToFunc() {
    throw RuntimeError("Number cannot be evaluated to function");
}
int Number::GetValue() const {
    return value_;
}

bool Boolean::GetValue() const {
    return value_;
}

const std::string& Symbol::GetName() const {
    return name_;
}

std::shared_ptr<Object> Symbol::EvalToFunc() {
    auto& map = OperationsMap::Instantiate();
    if (!map.CheckExistence(name_)) {
        return nullptr;
    }
    return map.GetOperation(name_);
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return first_;
}
std::shared_ptr<Object> Cell::GetSecond() const {
    return second_;
}

void Cell::SetFirst(std::shared_ptr<Object> other_first) {
    first_ = other_first;
}

void Cell::SetSecond(std::shared_ptr<Object> other_second) {
    second_ = other_second;
}

std::shared_ptr<Object> Quote::Apply(std::vector<std::shared_ptr<Object>>& args) {
    if (!args.empty()) {
        return args[0];
    }
}

template <typename Functor>
std::shared_ptr<Object> Calculate<Functor>::Apply(FuncArgs& args) {
    Functor f;
    int ans = SetDefaultValue(args);
    size_t start_index = SetStartIndex();
    if (!IsTypes<Number, Object>(args)) {
        throw RuntimeError("invalid type of arguments");
    }
    for (size_t i = start_index; i < args.size(); ++i) {
        int value = As<Number>(args[i])->GetValue();
        ans = f(ans, value);
    }
    return std::make_shared<Number>(ConstantToken{ans});
}

template <typename Functor>
int Calculate<Functor>::SetDefaultValue(FuncArgs& args) {
    if constexpr (std::is_same_v<Functor, PlusFunc>) {
        return 0;
    }
    if constexpr (std::is_same_v<Functor, MultFunc>) {
        return 1;
    }
    if (!args.empty()) {
        if (Is<Number>(args[0])) {
            return As<Number>(args[0])->GetValue();
        }
    }
    throw RuntimeError("invalid type of arguments");
}

template <typename Functor>
size_t Calculate<Functor>::SetStartIndex() {
    if constexpr (!std::is_same_v<Functor, MinusFunc> && !std::is_same_v<Functor, DivFunc>) {
        return 0;
    }
    return 1;
}

template <typename Functor>
std::shared_ptr<Object> Monotony<Functor>::Apply(FuncArgs& args) {
    Functor f;
    if (!IsTypes<Number>(args)) {
        throw RuntimeError("invalid type of arguments");
    }
    for (size_t i = 1; i < args.size(); ++i) {
        if (!Check(f, As<Number>(args[i - 1])->GetValue(), As<Number>(args[i])->GetValue())) {
            return std::make_shared<Boolean>(BooleanToken{false});
        }
    }
    return std::make_shared<Boolean>(BooleanToken{true});
}

std::shared_ptr<Object> Absolute::Apply(std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 1 && Is<Number>(args[0])) {
        int value = std::abs(As<Number>(args[0])->GetValue());
        return std::make_shared<Number>(ConstantToken{value});
    }
    throw RuntimeError("wrong type/number of arguments");
}

template <typename Functor>
bool Monotony<Functor>::Check(Functor functor, int first, int second) {
    return functor(first, second);
}

std::shared_ptr<Object> And::Apply(FuncArgs& args) {
    std::shared_ptr<Object> current;
    for (size_t i = 0; i < args.size(); ++i) {
        current = args[i];
        if (Is<Cell>(current)) {
            current = Unpack(current);
        }
        if (Is<Boolean>(current) && !As<Boolean>(current)->GetValue()) {
            return std::make_shared<Boolean>(BooleanToken{false});
        }
    }
    if (!args.empty()) {
        return current;
    }
    return std::make_shared<Boolean>(BooleanToken{true});
}

std::shared_ptr<Object> Or::Apply(FuncArgs& args) {
    std::shared_ptr<Object> current;
    for (size_t i = 0; i < args.size(); ++i) {
        current = args[i];
        if (Is<Cell>(current)) {
            current = Unpack(current);
        }
        if (!Is<Boolean>(current)) {
            return current;
        } else if (As<Boolean>(current)->GetValue()) {
            return std::make_shared<Boolean>(BooleanToken{true});
        }
    }
    if (!args.empty()) {
        return current;
    }
    return std::make_shared<Boolean>(BooleanToken{false});
}

std::shared_ptr<Object> Not::Apply(FuncArgs& args) {
    if (args.size() == 1) {
        bool value;
        if (Is<Boolean>(args[0])) {
            value = !As<Boolean>(args[0])->GetValue();
            return std::make_shared<Boolean>(BooleanToken{value});
        }
        return std::make_shared<Boolean>(BooleanToken{false});
    }
    throw RuntimeError("wrong type/number of arguments");
}

std::shared_ptr<Object> Cons::Apply(std::vector<std::shared_ptr<Object>>& args) {
    return std::make_shared<Cell>(args[0], args[1]);
}

std::shared_ptr<Object> Car::Apply(std::vector<std::shared_ptr<Object>>& args) {
    std::shared_ptr<Object> list = Unpack(args[0]);
    if (Is<Cell>(list)) {
        return As<Cell>(list)->GetFirst();
    }
    throw RuntimeError("Invalid arguments");
}

std::shared_ptr<Object> Cdr::Apply(std::vector<std::shared_ptr<Object>>& args) {
    std::shared_ptr<Object> list = Unpack(args[0]);
    if (Is<Cell>(list)) {
        return As<Cell>(list)->GetSecond();
    }
    throw RuntimeError("Invalid arguments");
}

std::shared_ptr<Object> MakeList::Apply(std::vector<std::shared_ptr<Object>>& args) {
    if (args.empty()) {
        return nullptr;
    }
    std::shared_ptr<Object> list = std::make_shared<Cell>(args[0], nullptr);
    std::shared_ptr<Object> next = list;
    for (size_t i = 1; i < args.size(); ++i) {
        As<Cell>(next)->SetSecond(std::make_shared<Cell>(args[i], nullptr));
        next = As<Cell>(next)->GetSecond();
    }
    return list;
}

std::shared_ptr<Object> ListRef::Apply(std::vector<std::shared_ptr<Object>>& args) {
    std::shared_ptr<Object> list = Unpack(args[0]);
    std::shared_ptr<Object> ans;
    size_t index;
    size_t cnt = 0;
    if (Is<Number>(args[1])) {
        index = As<Number>(args[1])->GetValue();
    } else {
        throw RuntimeError("Invalid index type");
    }
    while (Is<Cell>(list)) {
        if (cnt == index) {
            return As<Cell>(list)->GetFirst();
        }
        ++cnt;
        list = As<Cell>(list)->GetSecond();
    }
    if (cnt != index || !list) {
        throw RuntimeError("Invalid index value");
    }
}

std::shared_ptr<Object> ListTail::Apply(std::vector<std::shared_ptr<Object>>& args) {
    std::shared_ptr<Object> list = Unpack(args[0]);
    std::shared_ptr<Object> ans;
    size_t index;
    size_t cnt = 0;
    if (Is<Number>(args[1])) {
        index = As<Number>(args[1])->GetValue();
    } else {
        throw RuntimeError("Invalid index type");
    }
    while (Is<Cell>(list)) {
        if (cnt == index) {
            return As<Cell>(list);
        }
        ++cnt;
        list = As<Cell>(list)->GetSecond();
    }

    if (cnt == index) {
        return list;
    }
    throw RuntimeError("Invalid index value");
}

template <typename T>
std::shared_ptr<Object> Predicate<T>::Apply(FuncArgs& args) {
    return std::make_shared<Boolean>(BooleanToken{IsTypes<T, Object>(args)});
}

template <typename Functor>
std::shared_ptr<Object> ListPredicate<Functor>::Apply(std::vector<std::shared_ptr<Object>>& args) {
    Functor f;
    return std::make_shared<Boolean>(BooleanToken{f(args[0])});
}
