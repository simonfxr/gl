#ifndef LOG_HPP
#define LOG_HPP

namespace log {

static const unsigned LEVEL_COUNT = 5;

enum Level {
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

struct LogMessage {
    std::string& file;
    int line;
    std::string& function;
    LogMessage(const std::string& _file = "", int _line = -1, const std::string& _func = "") :
        file(_file), line(_line), function(_func)
        {}
};

struct LogTarget : public std::ostream {
    virtual bool ignore(Level lvl);
    virtual const LogTarget& route(Level lvl);
    virtual void setLogLevel(Level lvl) = 0;
    virtual std::ostream& beginLog(const LogMessage&) = 0;
    virtual void endLog();
};

struct DefaultLogTarget : public LogTarget {
    DefaultLogTarget();
    ~DefaultLogTarget();
    void route(Level lvl, ref::Ref<LogTarget>& dest);
    void ignore(Level lvl, bool ignored = true);
private:
    bool ignored[LEVEL_COUNT];
    ref::Ref<LogTarget> routing[LEVEL_COUNT];
};

struct LogOStream : public LogTarget {
    LogStream(ref::Ref<std::ostream>& dest);
    ~LogStream();
    
private:
    ref::Ref<std::ostream> dest;
};

#define LOG(src, lvl) log::begin(log::targetOf((src)), (lvl), log::LogMessage(__FILE__, __LINE, __FUNCTION__))

#define LOG_DEBUG(src) LOG(src, log::Level::Debug)
#define LOG_INFO(src) LOG(src, log::Level::Info)
#define LOG_WARN(src) LOG(src, log::Level::Warning)
#define LOG_ERROR(src) LOG(src, log::Level::Error)
#define LOG_FATAL(src) LOG(src, log::Level::Fatal)

template <typename T>
LogTarget& targetOf(const T&) {
    return global::target();
}

std::ostream& begin(const LogTarget& sink, Level lvl, const LogMessage& msg);

struct EndLogMessage {};

ostream& operator <<(ostream& out, const EndLogMessage&);

static const EndLogMessage end;

namespace global {

LogTarget& target();
void setTarget(LogTarget&);

} // namespace global

} // namespace log

#endif
