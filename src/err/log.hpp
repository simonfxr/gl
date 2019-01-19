#ifndef ERR_LOG_HPP
#define ERR_LOG_HPP

#include "err/err.hpp"

#include <type_traits>

#define LOG_RAISE(val, ec, lvl, msg)                                           \
    do {                                                                       \
        decltype(auto) PP_CAT(_log_raise_err_sender_, __LINE__) = (val);       \
        PP_CAT(_log_raise_err_sender_, __LINE__).pushError(ec);                \
        auto logmsg =                                                          \
          ::err::beginLog(PP_CAT(_log_raise_err_sender_, __LINE__), lvl);      \
        logmsg << msg << sys::io::endl;                                        \
    } while (0)

#define LOG_RAISE_ERROR(val, ec, msg)                                          \
    LOG_RAISE(val, ec, ::err::LogLevel::Error, msg)

namespace err {

template<typename OStream>
struct LogSettings
{
    OStream &out;
    LogLevel min_level;
};

template<typename OStream>
LogSettings<OStream>
makeLogSettings(OStream &out, LogLevel min_level = LogLevel::Info)
{
    return LogSettings<OStream>{ out, min_level };
}

template<typename T>
struct LogTraits
{
    // static LogSettings<OStream> getDestination(T &x);
};

template<typename OStream>
struct LogMessage
{
    OStream &_destination;
    bool _writeable;
    explicit operator bool() const { return _writeable; }

    template<typename T>
    LogMessage &append(T &&arg)
    {
        if (_writeable)
            _destination << std::forward<T>(arg);
        return *this;
    }

    template<typename T>
    friend LogMessage &operator<<(LogMessage &log, T &&arg)
    {
        return log.append(std::forward<T>(arg));
    }

    OStream &out() { return _destination; }
};

template<typename T>
static auto
beginLog(T &sender, LogLevel level = LogLevel::Info)
{
    const auto &settings = LogTraits<std::decay_t<T>>::getDestination(sender);
    return LogMessage<std::decay_t<decltype(settings.out)>>{
        settings.out, settings.out.writeable() && settings.min_level <= level
    };
}

} // namespace err

#endif
