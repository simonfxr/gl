#ifndef GE_VAR_HPP
#define GE_VAR_HPP

#include "ge/Command.hpp"

namespace ge {

typedef uint32 VarKind;

namespace {

const VarKind LOCAL_CONSTANT VAR_SET = 1;
const VarKind LOCAL_CONSTANT VAR_GET = 2;
const VarKind LOCAL_CONSTANT VAR_GETSET = VAR_SET | VAR_GET;

} // namespace anon

struct Var EXPLICIT : public Command {

    Var(VarKind kind = VAR_GETSET, CommandParamType type, const std::string& name, const std::string& desc);
    virtual ~Var();

    VarKind kind();
    CommandParamType type();

    virtual bool set(const CommandParam& val);
    virtual void get(CommandParam* val);

    void interactive(const Event<CommandEvent>&, const Array<CommandArg>&) override;

private:
    const VarKind _kind;
};

namespace impl {
template <typename T>
struct Meta;
} // namespace impl

template <typename T>
struct GenVar EXPLICIT : public Var {
private:
    T *var;
public:
    GenVar(T *v, const std::string& name, const std::string& desc) :
        Var(VAR_GETSET, impl::Meta<T>::type(), name, desc),
        var(v)
        {}
    const T& value() const { return *var; }
    T& value() { return *var; }
    void get(CommandArg *val) override {
        Meta<T>::get(value(), val);
    }
    bool set(const CommandArg& val) override {
        Meta<T>::set(value(), val);
        return true;
    }
};

template <typename T>
struct GenConstVar EXPLICIT: public Var {
private:
    const T *var;
public:
    GenVar(T *v, const std::string& name, const std::string& desc) :
        Var(VAR_GET, impl::Meta<T>::type(), name, desc),
        var(v)
        {}
    const T& value() const { return *var; }
    void get(CommandArg *val) override {
        Meta<T>::get(value(), val);
    }
}

namespace impl {

template <>
struct Meta<math::real> {
    static CommandParam type() {
        return NumberParam;
    }
    static void get(math::real var, CommandArg *val) {
        val->type = Number;
        val->number = var;
    }
    static void set(math::real& var, const CommandArg& val) {
        var = math::real(val.number);
    }
};

template <>
struct Meta<bool> {
    static CommandParam type() {
        return IntegerParam;
    }
    static void get(bool var, CommandArg *val) {
        val->type = IntegerParam;
        val->integer = var ? 1 : 0;
    }
    static void set(bool& var, const CommandArg& val) {
        var = val.integer != 0;
    }
};

} // namespace impl

} // namespace ge

#endif
