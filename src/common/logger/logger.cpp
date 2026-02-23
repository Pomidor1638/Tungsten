#include "Logger.h"
#include <algorithm>


Logger& Logger::instance()
{
    static Logger inst;
    return inst;
}

Logger& gLog = Logger::instance();

void Logger::addStream(std::ostream& os)
{
    std::lock_guard<std::mutex> lock(mtx);
    streams.push_back(&os);
}

void Logger::removeStream(std::ostream& os)
{
    std::lock_guard<std::mutex> lock(mtx);
    streams.erase(
        std::remove(streams.begin(), streams.end(), &os),
        streams.end()
    );
}
