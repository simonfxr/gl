#ifndef ERR_WITH_ERROR_HPP
#define ERR_WITH_ERROR_HPP

#include "defs.h"

#include <string>
#include <sstream>

namespace err {

template <typename E>
std::string defaultStringError(E error) {
    std::ostringstream rep;
    rep << error;
    return rep.str();
}

template <typename E, E NoError,  std::string(*StringErrorFun)(E) = defaultStringError<E> >
struct WithError {
    E lastError;

    WithError() : lastError(NoError) {}

    E getError() const { return lastError; }
    
    void pushError(E err) { if (lastError == NoError) lastError = err; }
    
    bool wasError() const { return lastError != NoError; }

    E clearError() { E err = lastError; lastError = NoError; return err; }

    static std::string stringError(E err) { return StringErrorFun(err); }
};

} // namespace err

#endif
