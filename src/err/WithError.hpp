#ifndef ERR_WITH_ERROR_HPP
#define ERR_WITH_ERROR_HPP

#include "defs.hpp"

#include <string>

namespace err {

template<typename E>
std::string
defaultStringError(E error)
{
    return std::to_string(int(error));
}

template<typename E,
         E NoError,
         std::string (*StringErrorFun)(E) = defaultStringError<E>>
struct WithError
{
    E lastError;

    WithError() : lastError(NoError) {}

    E getError() const { return lastError; }

    void pushError(E err)
    {
        if (lastError == NoError)
            lastError = err;
    }

    bool wasError() const { return lastError != NoError; }

    E clearError()
    {
        E err = lastError;
        lastError = NoError;
        return err;
    }

    static std::string stringError(E err) { return StringErrorFun(err); }
};

} // namespace err

#endif
