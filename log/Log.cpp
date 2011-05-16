#include "Log.hpp"

namespace log {

LogTarget begin(const LogTarget& sink, Level lvl, const LogMessage& msg) {
    
    if (lvl < 0 || lvl >= LEVEL_COUNT) {
        std::ostream& out = LOG_ERR(sink) << "invalid log::Level using log::Level::Error";
        if (!msg.file.empty()) {
            out << " at " << msg.file;
            if (msg.line != -1)
                out << ":" << line;
        }
        
        if (!msg.function.empty()) {
            out << " in " << msg.function << "()";
        }
        out << log::end;
        lvl = Level::Error;
    }
    
    const LogTarget *tgt = sink.target;
    while (!tgt->ignore(lvl)) {
        const LogTarget& newtgt = tgt->route(lvl);
        if (&newtgt == tgt) {
            tgt->beginLog(msg);
            return LogSink(*tgt);
        }
        tgt = &newtgt;
    }
}

} // namespace log

