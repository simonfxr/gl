#ifndef ERR_WITH_ERROR_HPP
#define ERR_WITH_ERROR_HPP

#include "defs.hpp"

#include <string>

namespace err {

template<typename E>
struct WithError
{
    E lastError;

    E getError() const { return lastError; }

    void pushError(E err)
    {
        if (lastError == E{})
            lastError = err;
    }

    bool wasError() const { return lastError != E{}; }

    E clearError()
    {
        E err = lastError;
        lastError = E{};
        return err;
    }
};

} // namespace err

#endif
