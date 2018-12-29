#include "ge/Var.hpp"

namespace ge {

static Array<CommandParamType> &
varParamArray(CommandParamType ty)
{
    static Array<CommandParamType> params[] = {
        PARAM_ARRAY(StringParam),  PARAM_ARRAY(IntegerParam),
        PARAM_ARRAY(NumberParam),  PARAM_ARRAY(VarRefParam),
        PARAM_ARRAY(CommandParam), PARAM_ARRAY(KeyComboParam),
        PARAM_ARRAY(AnyParam)
    };

    switch (ty) {
    case StringParam:
        return params[0];
    case IntegerParam:
        return params[1];
    case NumberParam:
        return params[2];
    case VarRefParam:
        return params[3];
    case CommandParam:
        return params[4];
    case KeyComboParam:
        return params[5];
    case AnyParam:
        return params[6];
    default:
        FATAL_ERR("Var::Var: invalid CommandParamType");
    }
}

template<typename T>
Array<T>
arraySingleton(const T &val)
{
    T *vals = new T[1];
    vals[0] = val;
    return Array<T>(vals, 1, Array::Owned);
}

Var::Var(VarKind k,
         CommandParamType type,
         const std::string &name,
         const std::string &desc)
  : Command(varParamArray(type), name, desc), _kind(k)
{}

~Var::Var() {}

virtual bool
Var::set(const CommandParam &val)
{
    if ((kind() & VAR_SET) == 0) {
        ERR("Var is not setable");
        return;
    }
    ERR("not implemented");
}

virtual void
Var::get(CommandParam *val)
{
    if ((kind() & VAR_GET) == 0) {
        ERR("Var is not getable");
        return;
    }
    ERR("not implemented");
}

void
Var::interactive(const Event<CommandEvent> &, const Array<CommandArg> &args)
{
    if ((kind() & VAR_SET) == 0) {
        ERR("Var is not setable");
        return;
    }

    if (!set(args[0]))
        ERR("setting value failed");
}

VarKind
Var::kind()
{
    return _kind;
}

CommandParamType
Var::type()
{
    return parameters()[0];
}

} // namespace ge
