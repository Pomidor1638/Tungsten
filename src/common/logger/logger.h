#pragma once
#include <ostream>
#include <vector>
#include <mutex>

class Logger {
public:
    static Logger& instance();

    void addStream(std::ostream& os);
    void removeStream(std::ostream& os);

    template<typename T>
    Logger& operator<<(const T& value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto* s : streams)
            (*s) << value;
        return *this;
    }

    using Manip = std::ostream& (*)(std::ostream&);
    Logger& operator<<(Manip manip)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto* s : streams)
            manip(*s);
        return *this;
    }

private:
    Logger() = default;
    std::vector<std::ostream*> streams;
    std::mutex mtx;
};

extern Logger& gLog;
